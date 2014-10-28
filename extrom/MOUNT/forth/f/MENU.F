.( MENU )

DEFER MENU_FILTER
' NOOP IS MENU_FILTER

0 value m_cursor

create m_spcs 128 allot

0 value m_x
0 value m_y
0 value m_addr
0 value m_rows
0 value m_cols
0 value m_menu_items
0 value m_rec_size

0 value m_offset
0 value m_selected
0 value m_update_cursor

0 value m_border_left
0 value m_border_right

: hborder_up ( addr in_size -- )
	>r 
	\ H# 97 over c!
	H# 9C over c!
	1+
	r> 0 do 
	   \ H# 83 over c!
	   H# 8C over c!
	   1+
	loop
	\ H# AB swap c!
	H# AC swap c!
;

: hborder_down ( addr in_size -- )
	>r 
	\ H# B5 over c!
	H# 8D over c!
	1+
	r> 0 do 
	   \ H# B0 over c!
	   H# 8C over c!
	   1+
	loop
	H# 8E swap c!
;

: hborder_lr ( addr in_size -- )
	>r 
	H# 95 over c!
	1+
	r> +
	\ r> 0 do 
	\    H# 80 over c!
	\    1+
    \ loop
	H# AA swap c!
;

: border ( x y dx dy -- )
 
  >r >r
  xy2aczu 
  r> 2dup hborder_up
  swap 64 +
  r> 0 do
    2dup swap hborder_lr
    64 +
  loop
  swap hborder_down
;




: update_menu ( using m_vars -- )
  m_spcs m_addr a.rec_size@ 1+  0 fill

  inv_off

  m_x 1- 
  m_y 1- 
  m_cols m_rec_size 2+  * 
  m_rows 


  vbl

  border

  vbl
  
  m_offset 0> if
	m_x 1 + m_y 1- xy2aczu 
	H# A4 over    c!
	h# A5 swap 1+ c!
  then

  m_offset m_rows m_cols * + m_menu_items < if
	m_x m_cols m_rec_size 2+ * + 2 - m_y m_rows +  xy2aczu 
	h# 89 over    c!
	h# A9 swap 1- c!
  then

  0 to m_update_cursor
  m_cols 0 do
    m_rows 0 do

  		m_update_cursor m_offset + 

  		dup m_selected = if
  			H# 10 to m_border_left
  			H# 11 to m_border_right
  		else
  			H# 00 to m_border_left
  			H# 00 to m_border_right
  		then

  		dup m_cursor = if
  			inv_on	
 		else
  			inv_off
 		then

  		m_addr a.records@ < if
  			m_addr m_update_cursor m_offset + a.buf[n]  
  		else
  			m_spcs
  		then                     

  		( straddr )
  		j 16 * m_x + i m_y + xy2aczu

  		m_border_left  over c!
  		m_border_right over m_addr a.rec_size@ 1+ + c!
  		
  		1+ m_addr a.rec_size@ move

  		m_update_cursor 1+ to m_update_cursor

  	loop
  loop
  inv_off
;

: mmplus0 ( value addval -- value ) 
	+ 
	dup m_offset - m_cols m_rows * 1- > if
    	dup m_menu_items < if 
    		m_offset 1+ to m_offset
    	then
	then
	m_menu_items 1- min
;

: mminus0 ( value addval -- value ) 
  - 
  dup m_offset - 0 < if
    m_offset 1- 0 max to m_offset
  then
  0 max
;

: mmplus  ( addval --  ) m_cursor swap 0 do 1 mmplus0 loop to m_cursor ;
: mminus ( addval -- ) m_cursor swap 0 do 1 mminus0 loop to m_cursor ;

: menu_select ( addr x y rows cols last_selected-- n/-1) 

 \ ." SELECT_IN: " .s


	dup to m_selected
	
	dup 0< if drop 0 then \ if last_selected == -1 (hide pointer)

	dup to m_cursor
	to m_offset

	to m_cols
	to m_rows
	to m_y
	to m_x
	
	dup to m_addr
	dup a.rec_size@ to m_rec_size
	a.records@ to m_menu_items

	m_cols m_rec_size 2+ * 64 > if

		." record_size   :" m_rec_size . cr
		." cols          :" m_cols  . cr
		." required_width=" m_cols m_rec_size 2+ * . cr

		abort" requred menu widht > 64"
	then


	\ 0 to m_offset
	\ 2 to m_selected
	
	-2 ( exit flag/ selected item, loop wile 0)
	begin
		update_menu


		1  15 gotoxy ." cursor=" 	m_cursor 	. ."  "
		15 15 gotoxy ." selected=" 	m_selected 	. ."  "
		30 15 gotoxy ." offset=" 	m_offset 	. ."  "
		45 15 gotoxy ." items=" 	m_menu_items . ."  "

		case 

			key 

			MENU_FILTER

			\ dup 55 2 gotoxy hex 2 .r decimal 
			
			h# 20 of m_cursor to m_selected    endof

			H# 88 of 1 		mminus endof \ up
			H# 82 of 1 		mmplus endof \ down
			H# 85 of 1 		mmplus endof \ menu 
			
			H# 84 of m_rows mminus endof \ left
			H# 86 of m_rows mmplus endof \ right

			H# 0D of drop m_cursor 				   endof \ enter
			H# 1B of drop -1 					   endof \ esc
		endcase

		dup -2 = not
	until
	0  15 gotoxy 63 spaces
	\ ." SELECT_OUT: " .s
;

behead m_spcs mminus

: filter_hex
	dup 55 2 gotoxy hex 2 .r decimal
;

\ ' filter_hex IS MENU_FILTER

false 
[if]
: stest
  scr_init
  binit
  test_buf 1 4 3  1 3 menu_select
;

: stest3
  scr_init
  binit
  test_buf 1 4 3  3 3 menu_select
;

: stest10
  scr_init
  binit
  test_buf 1 4 3 10 3 menu_select
;

[then]


\              ' ccc IS name
\              ' NOOP IS XXX


