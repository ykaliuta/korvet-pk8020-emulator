/*
 * AUTHOR: Sergey Erokhin                 esl@pisem.net,pk8020@gmail.com
 * &Korvet Team                                              2000...2005
 * ETALON Korvet Emulator                         http://pk8020.narod.ru
 * ---------------------------------------------------------------------
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#define pr_fmt(fmt) "Main: " fmt

#include "korvet.h"
#include "host.h"
#include "vg.h"
#include <assert.h>
#include "lan.h"

#ifdef DBG
#include "dbg/dbg.h"
#endif

struct main_ctx {
    bool quitting; /* Main loop exit flag */
    bool turbo;
    /* boost turbo in frames, (50*10) - 10 virtual second */
    unsigned turbo_boot_cnt;
    /* counts frame refreshes */
    int frame_counter;
    /*
     * the locks needed since the counters updated from timer thread
     * and the main thread. The increment operation is not atomic
     */
    host_mutex_t frame_counter_lock;
    /* periodically calculated frames per second */
    int fps;
    int counter50hz;
    host_mutex_t counter50hz_lock;
    host_cond_t counter50hz_cond;
    volatile bool sound_ready;
    host_cond_t sound_cond;
};

int verbose;

int VBLANK=0;
int SoundEnable=0;
int Takt;

int BW_Flag=0;

int main_loop_run_flag=1;

int turboBOOT; /* gets cmdline value here */

extern int KBD_LED;                  // RusEngFlag

extern byte SOUNDBUF[];
extern byte RAM[65535];
extern int KeyboadUpdateFlag;   // =1 if need rebuld keyboard layout
extern int KeyboardLayout;
extern int LutUpdateFlag;
extern int MuteFlag;
extern int AllScreenUpdateFlag;  // Флаг необходимости обновть весь экран

AUDIOSTREAM *stream;

extern const char AboutMSG[];

static int initial_scale;

static void main_50hz_tick(struct main_ctx *ctx);
static bool in_turbomode(struct main_ctx *ctx);

static void Timer_1S(void *c)
{
    struct main_ctx *ctx = c;

    ctx->fps = ctx->frame_counter;
    host_mutex_lock(&ctx->frame_counter_lock);
    ctx->frame_counter = 0;
    host_mutex_unlock(&ctx->frame_counter_lock);
}
END_OF_FUNCTION(Timer_1S);

static void Timer_50hz(void *c)
{
    struct main_ctx *ctx = c;

    host_mutex_lock(&ctx->counter50hz_lock);
    ctx->counter50hz++;
    host_mutex_unlock(&ctx->counter50hz_lock);
    host_cond_broadcast(&ctx->counter50hz_cond);
}

void Reset(void) {
    printf("Reset\n");
    GZU_Init();
    ACZU_Init();
    Memory_Init();
    FDC_Init();
    FDC_Reset();
    PIC_init();
    CPU_Init();
    KBD_Init();
    FDC_Reset();
    PPI_Init();
    Serial_Init();
    InitTMR();
    ResetOSD();
}

void trace_bios(word pc) {
    static unsigned int dma;
    word bc=CPU_GetBC();
    word HL;
    int i,j;
    switch (pc) {
        //DEFAULT cpm addrs
        case 0xda18: { printf("TRACE_BIOS: HOME\n"); break; }
        case 0xda1B: { printf("TRACE_BIOS: sel_DSK dsk=%d\n",bc & 0xff); break; }
        case 0xda1E: { printf("TRACE_BIOS: set_TRK trk=%d\n",bc & 0xff); break; }
        case 0xda21: { printf("TRACE_BIOS: set_SEC sec=%d\n",bc & 0xff); break; }
        case 0xda24: { printf("TRACE_BIOS: set_DMA dma=0x%04x\n",bc); break; }
        case 0xda27: { printf("TRACE_BIOS: READ\n"); break; }
        case 0xda2A: { printf("TRACE_BIOS: WRITE type=%d\n",bc & 0xff); break; }

        // microdos, DISKIO point
        case 0xE46E:  // IO 870430
        case 0xEAA3:  // IO 880630
        case 0xEAA5:  // IO 900105
        {
            HL=CPU_GetHL();
            dma=RAM[HL+6] | (RAM[HL+7] << 8);
            printf("DSK_IO: drv=%02x chword=%02x func=%02x numb=%02x trk=%02x sec=%02x abuf=%04x htrk=%02x hsec=%02x\n",
                RAM[HL+0],
                RAM[HL+1],
                RAM[HL+2],
                RAM[HL+3],
                RAM[HL+4],
                RAM[HL+5],
                dma,
                RAM[HL+8],
                RAM[HL+9]
            );
            break;
        }

        case 0xeabe:  //dma buf after IO
        {
            printf("DMA BUF:\n");
            for (i=0;i<8;i++) {
                printf("%04X: ",dma+i*16);
                for (j=0;j<16;j++) {
                    byte v=RAM[dma+i*16+j];
                    printf("%02X ",v);
                }
                printf("\n");
            }
        }
    }
}

static void trace_tick()
{
#ifdef DBG
    word  __PC=CPU_GetPC();

    if (__PC == 0x0000) CheckROM();
    else if (__PC == 0xc75c) CheckCCP();
    else if (__PC == 0xcade) CheckComEXEC();

     /* show special bios events like disk access */
     /* trace_bios(__PC); */

#endif
}

static bool turbo_key_pressed(struct main_ctx *ctx)
{
    return key[KEY_F6] != 0;
}

static bool in_turboboot(struct main_ctx *ctx)
{
    return ctx->turbo_boot_cnt > 0;
}

static void turboboot_tick(struct main_ctx *ctx)
{
    ctx->turbo_boot_cnt--;
}

/* returns if the mode has been just changed */
static bool turbomode_set(struct main_ctx *ctx, bool enable)
{
    bool old = ctx->turbo;

    ctx->turbo = enable;
    return old != ctx->turbo;
}

static bool turboboot_disable(struct main_ctx *ctx)
{
    ctx->turbo_boot_cnt = 0;
    return turbomode_set(ctx, false);
}

static bool in_turbomode(struct main_ctx *ctx)
{
    return ctx->turbo;
}

static bool turbomode_update(struct main_ctx *ctx)
{
    bool turbo_key = turbo_key_pressed(ctx);

    if (in_turboboot(ctx) && turbo_key) {
        pr_vdebug("Turbo key pressed, disabling turbo boot\n");
        return turboboot_disable(ctx);
    }

    if (in_turboboot(ctx)) { /* and no KEY */
        turboboot_tick(ctx);
        return false;
    }

    /* normal turbo mode, enabled while the key is pressed */
    return turbomode_set(ctx, turbo_key);
}

static bool turbo_enabling(struct main_ctx *ctx, bool changed)
{
    return changed && in_turbomode(ctx);
}

#ifdef SOUND

static void sound_mute_set(bool enable)
{
    MuteFlag = enable;
}

static void main_wait_sound_ready(struct main_ctx *ctx)
{
    host_mutex_t m = HOST_MUTEX_INITIALIZER;

    /*
     * No need for the mutex here,
     * operations on sound_ready are atomic.
     */
    host_mutex_lock(&m);
    while (!ctx->sound_ready)
        host_cond_wait(&ctx->sound_cond, &m);
    host_mutex_unlock(&m);
}

static void sound_callback(uint8_t *p, unsigned len, void *a)
{
    struct main_ctx *ctx = a;

    main_wait_sound_ready(ctx);

    memcpy(p,SOUNDBUF,AUDIO_BUFFER_SIZE);
#ifdef WAV
    AddWAV(p,AUDIO_BUFFER_SIZE);
#endif
    ctx->sound_ready = false;
}

static void sound_update(bool in_turbo, struct main_ctx *ctx)
{
    sound_mute_set(in_turbo);
    /* It updates counter's buffer even on mute */
    MakeSound();
    ctx->sound_ready = true;
    host_cond_broadcast(&ctx->sound_cond);
}

#else
static inline void sound_update(bool in_turbo, struct main_ctx *ctx) {};
static void sound_mute_set(bool enable) {};
#endif

static void main_ctx_prepare(struct main_ctx *ctx)
{
    ctx->turbo_boot_cnt = turboBOOT;

    if (in_turboboot(ctx)) {
        pr_vdebug("Turbo boot enabled (%d), enabling turbo mode\n",
                  turboBOOT);
        turbomode_set(ctx, true);
        sound_mute_set(true);
    }
}

static bool turbo_update(struct main_ctx *ctx)
{
    bool turbo_changed;

    turbo_changed = turbomode_update(ctx);

    if (turbo_enabling(ctx, turbo_changed))
        AllScreenUpdateFlag=1;

    sound_update(in_turbomode(ctx), ctx);

    return in_turbomode(ctx);
}

typedef void (*key_handler)(struct main_ctx *ctx, int key);

static void handle_quit(struct main_ctx *ctx, int key)
{
    ctx->quitting = true;
}

static void handle_dump(struct main_ctx *ctx, int key)
{
    Write_Dump();
}

static void handle_scale(struct main_ctx *ctx, int key)
{
    SCREEN_IncScale(); /* will update the screen */
    update_osd(ctx->fps);
}

static void handle_debug_lut_start(struct main_ctx *ctx, int key)
{
    Debug_LUT_start();
}
static void handle_debug_lut_end(struct main_ctx *ctx, int key)
{
    Debug_LUT_end();
}

static void handle_dbg(struct main_ctx *ctx, int key)
{
#ifdef DBG
    dbg_schedule_run();
#endif
}

static void handle_gui(struct main_ctx *ctx, int key)
{
    GUI();
}

static void handle_bw(struct main_ctx *ctx, int key)
{
    BW_Flag ^= 1;
    LutUpdateFlag = 1;
}

static void handle_reset(struct main_ctx *ctx, int key)
{
    Takt=0;
    Reset();
}

static key_handler keys_nomods[KEY_MAX] = {
    [KEY_F7] = handle_debug_lut_start,
    [KEY_F8] = handle_scale,
    [KEY_F9] = handle_dbg,
    [KEY_F10] = handle_bw,
    [KEY_F11] = handle_reset,
    [KEY_F12] = handle_quit,
};

static key_handler keys_up_nomods[KEY_MAX] = {
    [KEY_F7] = handle_debug_lut_end,
};
static key_handler keys_alt[KEY_MAX] = {
    [KEY_F8] = handle_dump,
    [KEY_F9] = handle_gui,
};

static key_handler *key_handlers[HOST_KEY_MAX][HOST_MOD_MAX] = {
    [HOST_KEY_DOWN][HOST_MOD_NONE] = keys_nomods,
    [HOST_KEY_DOWN][HOST_MOD_ALT] = keys_alt,
    [HOST_KEY_UP][HOST_MOD_NONE] = keys_up_nomods,
};

static void process_events(struct main_ctx *ctx)
{
    struct host_event ev;
    key_handler *handlers = NULL;
    key_handler handler = NULL;

    if (!host_event_pop(&ev))
        return;

    switch (ev.type) {
    case HOST_KEY_DOWN:
    case HOST_KEY_UP:
        if (ev.key.mods == 0)
            handlers = key_handlers[ev.type][HOST_MOD_NONE];
        else if (BIT_TEST(ev.key.mods, HOST_MOD_ALT))
            handlers = key_handlers[ev.type][HOST_MOD_ALT];

        if (handlers != NULL)
            handler = handlers[ev.key.code];

        if (handler != NULL)
            handler(ctx, ev.key.code);
        else
            KBD_update(ev.key.code, ev.type == HOST_KEY_DOWN);

        break;

    case HOST_MOUSE:
        MouseUpdate(ev.mouse.buttons, ev.mouse.dx, ev.mouse.dy);
        break;

    case HOST_JOYSTICK:
        Joystick_Update(ev.joystick.buttons, ev.joystick.axis);
        break;

    default:
        pr_error("Unhandled event\n");
    }
}

static bool main_quitting(struct main_ctx *ctx)
{
    return ctx->quitting;
}

static void main_50hz_tick(struct main_ctx *ctx)
{
    host_mutex_lock(&ctx->counter50hz_lock);
    ctx->counter50hz = 0;
    host_mutex_unlock(&ctx->counter50hz_lock);

    PIC_IntRequest(4);

    TimerTrace("V: %08d\n",Takt);

    if (LutUpdateFlag) LUT_Update(BW_Flag);
    SCREEN_ShowScreen();

    host_mutex_lock(&ctx->frame_counter_lock);
    ctx->frame_counter++;
    host_mutex_unlock(&ctx->frame_counter_lock);

    Takt-=ALL_TAKT;

    update_osd(ctx->fps);

    // update_rus_lat();
}

static void main_wait_50hz_tick(struct main_ctx *ctx)
{
    while (!ctx->counter50hz) {
        /*
         * We do not really need the lock here,
         * reading aboveis atomic, so use is for cond_wait only.
         */
        host_mutex_lock(&ctx->counter50hz_lock);
        host_cond_wait(&ctx->counter50hz_cond, &ctx->counter50hz_lock);
        host_mutex_unlock(&ctx->counter50hz_lock);
    }
}

static void main_loop(struct main_ctx *ctx)
{
    /* hopefully will be moved to some local context */
    Takt=0;

    main_ctx_prepare(ctx);

    for (;;) {
        process_events(ctx);

        /* Exit here */
        if (main_quitting(ctx))
            break;

        trace_tick(); /* defined above */
        /* can start debugger if hits breakpoint */
        dbg_tick();

        Takt+=CPU_Exec1step();

        LAN_poll();

        if (Takt>=ALL_TAKT) {
            Timer50HzTick();

            turbo_update(ctx);
            if (!in_turbomode(ctx))
                main_wait_50hz_tick(ctx);

            main_50hz_tick(ctx);
        }
    }
}

void main_set_initial_scale(int s)
{
    initial_scale = s;
}

static int do_hw_inits(struct main_ctx *ctx) {
    #ifdef DBG
    dbg_INIT();
    #endif

    set_uformat(U_ASCII);

    install_keyboard();
    install_timer();
    install_mouse();
    enable_hardware_cursor();
    show_os_cursor(MOUSE_CURSOR_QUESTION);


    LOCK_FUNCTION(Timer_50hz);
    LOCK_VARIABLE(Counter50hz);

    LOCK_FUNCTION(Timer_1S);
    LOCK_VARIABLE(FPS);
    LOCK_VARIABLE(ShowedLines_Scr);
    LOCK_VARIABLE(ShowedLines);
    LOCK_VARIABLE(ShowedLinesTotal);
    LOCK_VARIABLE(ShowedLinesCnt);


    if (host_sound_init() < 0)
        pr_error("Could not init sound\n");

    if (host_sound_start(AUDIO_BUFFER_SIZE, SOUNDFREQ, sound_callback, ctx) < 0)
        pr_error("Could not start sound\n");

    host_timer_start_bps(1, Timer_1S, ctx);
    host_timer_start_bps(50, Timer_50hz, ctx);

    return 0;
}

static void main_ctx_init(struct main_ctx *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    host_mutex_init(&ctx->frame_counter_lock);
    host_mutex_init(&ctx->counter50hz_lock);
    host_cond_init(&ctx->counter50hz_cond);
    host_cond_init(&ctx->sound_cond);
};

static void main_ctx_destroy(struct main_ctx *ctx)
{
    host_cond_destroy(&ctx->sound_cond);
    host_cond_destroy(&ctx->counter50hz_cond);
    host_mutex_destroy(&ctx->frame_counter_lock);
    host_mutex_destroy(&ctx->counter50hz_lock);
}

int main(int argc,char **argv) {

    int i;
    struct main_ctx ctx;

    //LUT_BASE_COLOR = 0x80
    assert(LUT_BASE_COLOR == 0x80); //very important for SCREEN_ShowScreen

    main_ctx_init(&ctx);

    printf("%s\n",AboutMSG);

    allegro_init();

    ReadConfig(); // Read KORVET.CFG file and set default values ...

    // parse command line option -A filename -B filename
    parse_command_line(argc,argv);

    check_missing_images();


    InitOSD();
    InitPrinter();

    LAN_Init();

    InitTimer();

    #ifdef WAV
    OpenWAV("korvet.wav");
    #endif

    SCREEN_Init(initial_scale);

    i=do_hw_inits(&ctx);
    if (i != 0) return i;

    host_init();

    /* Joystick must come after host, because of the event queue */
    if (JoystickEnabled) {
        if (host_joystick_init(JoystickNumber) < 0)
            exit(1);
    }

    fflush(stdout);
    SCREEN_SetGraphics(SCR_EMULATOR);

    LUT_Init();
    LUT_Update(BW_Flag);

    Reset();

    main_loop(&ctx);

    #ifdef WAV
    CloseWAV();
    #endif

    DestroyOSD();
    DestroyPrinter();
    DestroyTimer();
    SCREEN_destroy();

    host_joystick_shutdown();
    host_shutdown();
    allegro_exit();

    LAN_destroy();

    main_ctx_destroy(&ctx);

    return 0;
}
END_OF_MAIN();
