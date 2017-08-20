RUBPATH  = ../tools/rubbish/
MAKE     = make -r
NASK     = ../tools/nask
CC1      = ../tools/gocc1 -I$(RUBPATH) -Os -Wall -quiet
GAS2NASK = ../tools/gas2nask -a
OBJ2BIM  = ../tools/obj2bim
BIM2RUB  = ../tools/bim2rub
BIN2OBJ  = ../tools/bin2obj
RULEFILE = ../tools/rubbish/rubbish.rul
EDIMG    = ../tools/edimg
IMGTOL   = ../tools/imgtol
DEL      = rm -f
MAKEFONT = ../tools/makefont
OBJS_API = api001.obj api002.obj api003.obj api004.obj api005.obj api006.obj api007.obj api008.obj api009.obj api010.obj api011.obj api012.obj api013.obj api014.obj api015.obj api016.obj api017.obj api018.obj api019.obj
GOLIB    = ../tools/golib00

apilib.lib : Makefile $(OBJS_API)
	$(GOLIB) $(OBJS_API) out:apilib.lib

default :
	$(MAKE) img

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

clean :
	$(DEL) *.lst
	$(DEL) *.obj

src_only :
	$(MAKE) clean
	$(DEL) apilib.lib
