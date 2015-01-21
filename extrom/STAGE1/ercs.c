//  Утилита предназначена для рассчета контрольной суммы файлов, загружаемых 
//  ОПТС в корвет из внешнего ПЗУ, подключенного к боковому разъему.
//    Имя файла передается в командной строке
//  Файл обрезается на границе блока 256 байт, и контрольная сумма вписывается в последний байт файла
//
#include <stdio.h>
#include <errno.h>

void main(int argc,char* argv[]) {

FILE* in=fopen(argv[1],"r+");

int i;
unsigned char csum=0;
unsigned char buf[32768];
unsigned char* dptr=buf;
unsigned int scount;

if (in == 0) {
 printf("\nФайл не найден\n");
 return;
} 
// Подсчет числа блоков в файле
for(scount=0;;scount++) {
  fread(dptr,1,256,in);
  if (feof(in)) break;
  dptr+=256;
}
printf("\n Размер файла - %i блоков,",scount);
scount*=256;  // получаем размер файла в байтах
printf(" %i байт",scount);

for(i=0;i<(scount-1);i++) csum+=buf[i];
csum=0xff-csum;
printf("\n Контрольная сумма - %02x\n",csum);
fseek(in,scount-1,SEEK_SET);   // встаем на последний байт файла
fwrite(&csum,1,1,in);          // записываем КС
ftruncate(fileno(in),scount);  // обрезаем файл на границе блока
fclose(in);
}
