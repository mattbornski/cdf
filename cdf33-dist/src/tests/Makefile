#------------------------------------------------------------------------------
#
#  NSSDC/CDF						CDF tests makefile.
#
#  Version 1.5, 20-Feb-96, Hughes STX.
#
#  Modification history:
#
#   V1.0  26-Jan-94, J Love	Original version.
#   V1.0a 23-Feb-94, J Love	Added `CC='.
#   V1.0b  6-Apr-94, J Love	Finished `CC='.
#   V1.1  19-Dec-94, J Love	CDF V2.5.
#   V1.2   5-Jan-95, J Love	Updated dependencies, etc.
#   V1.2a 20-Jan-95, J Love	IRIX 6.x.
#   V1.3  16-Mar-95, J Love	Multi-compiler support, etc.
#   V1.4   3-Apr-95, J Love	POSIX.
#   V1.5  20-Feb-96, J Love	CDF V2.6.
#   V1.6  12-Feb-07, D Berger   Added existing LD_LIBRARY_PATH to temp 
#                               definition.
#
#------------------------------------------------------------------------------
#
# Notes:
#
# `make' on DECstations complains (prints an error message) if an if-then-else
# construct evaluates to FALSE but there is no `false' part.  To prevent this,
# the `NULL' command is executed as the `false' part.
#
#------------------------------------------------------------------------------

.SILENT:

SHELL=/bin/sh
NULL=true

FORTRAN=
FCx=
CCx=
SHARED=
FOPTIONS=
FOPTIONSld=
COPTIONS=
COPTIONSld=
SYSLIBS=
SHAREDEXT=

INCLUDEcdf=../include

CC=$(CCx)
CFLAGS=$(COPTIONS) -I$(INCLUDEcdf)
CFLAGSld=$(COPTIONSld)

FC=$(FCx)
FFLAGS=$(FOPTIONS)
FFLAGSld=$(FOPTIONSld)
MACos=Darwin

LIBCDFa=../lib/libcdf.a
LIBCDFshare=../lib/libcdf.$(SHAREDEXT)

LIBsARCHIVE=$(LIBCDFa) $(SYSLIBS)
LIBsSHARE=$(LIBCDFshare) $(SYSLIBS)

CDFh=$(INCLUDEcdf)/cdf.h
CDFinc=$(INCLUDEcdf)/cdf.inc

BINsC=qst2c qst2ic qst2icEPOCH16 qst2cNew
BINsF=qst2f qst2if qst2ifEPOCH16 qst2fNew
BINsCshare=qst2cS qst2icS qst2icEPOCH16S qst2cNewS
BINsFshare=qst2fS qst2ifS qst2ifEPOCH16S qst2ifSNew

#------------------------------------------------------------------------------
# Compile/link.
#------------------------------------------------------------------------------

all: c.yes cS.$(SHARED) f.$(FORTRAN) fS.$(FORTRAN).$(SHARED)

%.o : %.mod

c.yes: $(BINsC)

cS.yes: $(BINsCshare)
cS.no:

f.yes: $(BINsF)
f.no:

fS.yes.yes: $(BINsFshare)
fS.yes.no:
fS.no.yes:
fS.no.no:

qst2c:	qst2c.o $(LIBCDFa)
	@echo $(CC) $(CFLAGSld) -o $@ qst2c.o $(LIBsARCHIVE)
	      $(CC) $(CFLAGSld) -o $@ qst2c.o $(LIBsARCHIVE)

qst2cNew:	qst2cNew.o $(LIBCDFa)
	@echo $(CC) $(CFLAGSld) -o $@ qst2cNew.o $(LIBsARCHIVE)
	      $(CC) $(CFLAGSld) -o $@ qst2cNew.o $(LIBsARCHIVE)

qst2ic:	qst2ic.o $(LIBCDFa)
	@echo $(CC) $(CFLAGSld) -o $@ qst2ic.o $(LIBsARCHIVE)
	      $(CC) $(CFLAGSld) -o $@ qst2ic.o $(LIBsARCHIVE)

qst2icEPOCH16: qst2icEPOCH16.o $(LIBCDFa)
	@echo $(CC) $(CFLAGSld) -o $@ qst2icEPOCH16.o $(LIBsARCHIVE)
	      $(CC) $(CFLAGSld) -o $@ qst2icEPOCH16.o $(LIBsARCHIVE)

qst2cS: qst2c.o $(LIBCDFshare)
	@echo $(CC) $(CFLAGSld) -o $@ qst2c.o $(LIBsSHARE)
	      $(CC) $(CFLAGSld) -o $@ qst2c.o $(LIBsSHARE)

qst2cNewS: qst2cNew.o $(LIBCDFshare)
	@echo $(CC) $(CFLAGSld) -o $@ qst2cNew.o $(LIBsSHARE)
	      $(CC) $(CFLAGSld) -o $@ qst2cNew.o $(LIBsSHARE)

qst2icS: qst2ic.o $(LIBCDFshare)
	@echo $(CC) $(CFLAGSld) -o $@ qst2ic.o $(LIBsSHARE)
	      $(CC) $(CFLAGSld) -o $@ qst2ic.o $(LIBsSHARE)

qst2icEPOCH16S: qst2icEPOCH16.o $(LIBCDFa)
	@echo $(CC) $(CFLAGSld) -o $@ qst2ic.o $(LIBsSHARE)
	      $(CC) $(CFLAGSld) -o $@ qst2icEPOCH16.o $(LIBsSHARE)

qst2f:	qst2f.o $(LIBCDFa)
	@if [ $(MACos) =  `uname` ] ; then \
	   if [ `uname -r | cut -f1 -d.` = 8 ]; then \
	     echo $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsARCHIVE) -lSystemStubs; \
	          $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsARCHIVE) -lSystemStubs; \
	   else \
	     echo $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsARCHIVE); \
	          $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsARCHIVE); \
	   fi \
	else \
	  echo $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsARCHIVE); \
	       $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsARCHIVE); \
	fi
#	@echo $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsARCHIVE)
#	      $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsARCHIVE)

qst2if:	qst2if.o $(LIBCDFa)
	@if [ $(MACos) =  `uname` ] ; then \
	   if [ `uname -r | cut -f1 -d.` = 8 ]; then \
	     echo $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsARCHIVE) -lSystemStubs; \
	          $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsARCHIVE) -lSystemStubs; \
	   else \
	     echo $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsARCHIVE); \
	          $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsARCHIVE); \
	   fi \
	else \
	  echo $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsARCHIVE); \
	       $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsARCHIVE); \
	fi
#	@echo $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsARCHIVE)
#	      $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsARCHIVE)

qst2fNew: qst2fNew.o $(LIBCDFa)
	@if [ $(MACos) =  `uname` ] ; then \
	   if [ `uname -r | cut -f1 -d.` = 8 ]; then \
	     echo $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsARCHIVE) -lSystemStubs; \
	          $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsARCHIVE) -lSystemStubs; \
	   else \
	     echo $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsARCHIVE); \
	          $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsARCHIVE); \
	   fi \
	else \
	  echo $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsARCHIVE); \
	       $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsARCHIVE); \
	fi
#	@echo $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsARCHIVE)
#	      $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsARCHIVE)

qst2ifEPOCH16: qst2ifEPOCH16.o $(LIBCDFa)
	@if [ $(MACos) =  `uname` ] ; then \
	   if [ `uname -r | cut -f1 -d.` = 8 ]; then \
	     echo $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsARCHIVE) -lSystemStubs; \
	          $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsARCHIVE) -lSystemStubs; \
	   else \
	     echo $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsARCHIVE); \
	          $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsARCHIVE); \
	   fi \
	else \
	  echo $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsARCHIVE); \
	       $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsARCHIVE); \
	fi
#	@echo $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsARCHIVE)
#	      $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsARCHIVE)

qst2fS:	qst2f.o $(LIBCDFshare)
	@if [ $(MACos) =  `uname` ] ; then \
	   if [ `uname -r | cut -f1 -d.` = 8 ]; then \
	     echo $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsSHARE) -lSystemStubs; \
	          $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsSHARE) -lSystemStubs; \
	   else \
	     echo $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsSHARE); \
	          $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsSHARE); \
	   fi \
	else \
	  echo $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsSHARE); \
	       $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsSHARE); \
	fi
#	@echo $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsSHARE)
#	      $(FC) $(FFLAGSld) -o $@ qst2f.o $(LIBsSHARE)

qst2ifS: qst2if.o $(LIBCDFshare)
	@if [ $(MACos) =  `uname` ] ; then \
	   if [ `uname -r | cut -f1 -d.` = 8 ]; then \
	     echo $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsSHARE) -lSystemStubs; \
	          $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsSHARE) -lSystemStubs; \
	   else \
	     echo $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsSHARE); \
	          $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsSHARE); \
	   fi \
	else \
	  echo $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsSHARE); \
	       $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsSHARE); \
	fi
#	@echo $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsSHARE)
#	      $(FC) $(FFLAGSld) -o $@ qst2if.o $(LIBsSHARE)

qst2ifSNew: qst2fNew.o $(LIBCDFshare)
	@if [ $(MACos) =  `uname` ] ; then \
	   if [ `uname -r | cut -f1 -d.` = 8 ]; then \
	     echo $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsSHARE) -lSystemStubs; \
	          $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsSHARE) -lSystemStubs; \
	   else \
	     echo $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsSHARE); \
	          $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsSHARE); \
	   fi \
	else \
	  echo $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsSHARE); \
	       $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsSHARE); \
	fi
#	@echo $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsSHARE)
#	      $(FC) $(FFLAGSld) -o $@ qst2fNew.o $(LIBsSHARE)

qst2ifEPOCH16S: qst2ifEPOCH16.o $(LIBCDFshare)
	@if [ $(MACos) =  `uname` ] ; then \
	   if [ `uname -r | cut -f1 -d.` = 8 ]; then \
	     echo $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsSHARE) -lSystemStubs; \
	          $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsSHARE) -lSystemStubs; \
	   else \
	     echo $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsSHARE); \
	          $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsSHARE); \
	   fi \
	else \
	  echo $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsSHARE); \
	       $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsSHARE); \
	fi
#	@echo $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsSHARE)
#	      $(FC) $(FFLAGSld) -o $@ qst2ifEPOCH16.o $(LIBsSHARE)

.c.o:
	@echo $(CC) $(CFLAGS) -c $<
	      $(CC) $(CFLAGS) -c $<

.f.o:
	@cp $(CDFinc) CDF.INC
	@echo $(FC) $(FFLAGS) -c $<
	      $(FC) $(FFLAGS) -c $<
	@rm CDF.INC

qst2c.o: qst2c.c $(CDFh)
qst2cNew.o: qst2cNew.c $(CDFh)
qst2ic.o: qst2ic.c $(CDFh)
qst2icEPOCH16.o: qst2icEPOCH16.c $(CDFh)
qst2f.o: qst2f.f $(CDFh) $(CDFinc)
qst2if.o: qst2if.f $(CDFh) $(CDFinc)
qst2fNew.o: qst2fNew.f $(CDFh) $(CDFinc)
qst2ifEPOCH16.o: qst2ifEPOCH16.f $(CDFh) $(CDFinc)

#------------------------------------------------------------------------------
# Tests.
#     Each executable name is preceeded by `./' in case the current directory
# is not in the path.
#------------------------------------------------------------------------------

test:
	@if [ -f TEST.cdf ] ; then \rm TEST.* ; else $(NULL) ; fi
	@echo "Testing CDF library..."
	@./qst2c
	@./qst2cNew
	@./qst2ic
	@./qst2icEPOCH16
	@if [ -f qst2f ] ; then ./qst2f ; else $(NULL) ; fi
	@if [ -f qst2if ] ; then ./qst2if ; else $(NULL) ; fi
	@if [ -f qst2fNew ] ; then ./qst2fNew ; else $(NULL) ; fi
	@if [ -f qst2ifEPOCH16 ] ; then ./qst2ifEPOCH16 ; else $(NULL) ; fi
	@if [ -f qst2cS ] ; then echo "Testing shareable CDF library..." ; \
			    else $(NULL) ; \
	 fi
	@if [ -f qst2cS ] ; then make test.qst2cS ; else $(NULL) ; fi
	@if [ -f qst2cNewS ] ; then make test.qst2cNewS ; else $(NULL) ; fi
	@if [ -f qst2icS ] ; then make test.qst2icS ; else $(NULL) ; fi
	@if [ -f qst2icEPOCH16S ] ; then make test.qst2icEPOCH16S ; else $(NULL) ; fi
	@if [ -f qst2fS ] ; then make test.qst2fS ; else $(NULL) ; fi
	@if [ -f qst2ifS ] ; then make test.qst2ifS ; else $(NULL) ; fi
	@if [ -f qst2ifSNew ] ; then make test.qst2ifSNew ; else $(NULL) ; fi
	@if [ -f qst2ifEPOCH16S ] ; then make test.qst2ifEPOCH16S ; else $(NULL) ; fi

test.qst2cS:
	@env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):`pwd`/../lib env DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):`pwd`/../lib ./qst2cS

test.qst2cNewS:
	@env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):`pwd`/../lib env DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):`pwd`/../lib ./qst2cNewS

test.qst2icS:
	@env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):`pwd`/../lib env DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):`pwd`/../lib ./qst2icS

test.qst2icEPOCH16S:
	@env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):`pwd`/../lib env DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):`pwd`/../lib ./qst2icEPOCH16S

test.qst2fS:
	@env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):`pwd`/../lib env DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):`pwd`/../lib ./qst2fS
test.qst2ifS:
	@env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):`pwd`/../lib env DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):`pwd`/../lib ./qst2ifS

test.qst2ifSNew:
	@env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):`pwd`/../lib env DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):`pwd`/../lib ./qst2ifSNew

test.qst2ifEPOCH16S:
	@env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):`pwd`/../lib env DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):`pwd`/../lib ./qst2ifEPOCH16S

#------------------------------------------------------------------------------

#------------------------------------------------------------------------------

clean:
	@-rm -f *.o
	@-rm -f qst2c
	@-rm -f qst2cNew
	@-rm -f qst2ic
	@-rm -f qst2iEPOCH16c
	@-rm -f qst2f
	@-rm -f qst2if
	@-rm -f qst2fNew
	@-rm -f qst2iEPOCH16f
	@-rm -f qst2cS
	@-rm -f qst2cNewS
	@-rm -f qst2icS
	@-rm -f qst2icEPOCH16S
	@-rm -f qst2fS
	@-rm -f qst2ifS
	@-rm -f qst2ifSNew
	@-rm -f qst2ifEPOCH16S
	@-rm -f *.trace
	@-rm -f core

purge:
	@-rm -f *~
	@-rm -f *#
