HEADERS =  barrier bitvec case ckd_alloc cmd_ln err filename glist \
s3types bio vector logs3 cont_mgau subvq threading mdef dict dict2pid fillpen \
lm wid tmat kbcore hmm hyp lextree vithist ascr beam kb corpus utt new_fe_sp \
new_fe cmn cmn_prior agc feat live hash heap io libutil prim_type profile \
str2words unlimit cmd_ln_args

SRC =  barrier bitvec case ckd_alloc cmd_ln err filename glist bio vector \
logs3 hash heap io profile str2words unlimit parse_args_file cont_mgau subvq \
threading mdef dict dict2pid fillpen lm wid tmat kbcore hmm lextree vithist \
ascr beam kb corpus utt new_fe_sp new_fe cmn cmn_prior agc feat live \
main_live_pretend

TARGET = livepretend

# To see debug messages, set USE_DBG to 1
USE_DBG =0 

# To enable multithreading, uncomment USE_THRD. THREADS determines the number
# of threads to be created. PHASES is a bitwise specification of the different
# phases to be threaded. Bit 0 is Gaussian model evaluation, bit 1 is 
# lexical tree evaluation, bit 2 is lexical tree propagation. 7 means enabling
# threading in all 3 phases.
USE_THRD = -DTHRD
THREADS =1
PHASES =7 

# To enable SSE2, uncomment the following line
USE_SSE2 = -use_msasm -DSSE2

KILL_LOOP = -DLOGS3_NO_LOOP -DMGAU_NO_LOOP

USERFLAGS =  $(KILL_LOOP) -DDEBUG=$(USE_DBG) -DAD_BACKEND_OSS \
$(USE_THRD)  -DNUM_THREADS=$(THREADS) -DPHASES=$(PHASES) -DNEW_EVAL_LOGS3 

USERLIBS = -lm -lpthread 

LIBDIR = /usr/lib

# Uncomment USE_PENTIUM_4 if you are using this processor
# Can also profile the code by using USE_GPROF

USE_PENTIUM_4 = -vec- -march=pentium4 -mcpu=pentium4 
#USE_GPROF = -p

# Choose the appropriate compiler gcc or Intel icc

CC = gcc
LD = gcc       # Using GCC as the compiler for now
OPTIMIZE = -g -O4 #-Wall

#CC =  /opt/intel_cc_80/bin/icc
#LD = /opt/intel_cc_80/bin/icc       # Using GCC as the compiler for now
#OPTIMIZE = $(USE_SSE2) $(USE_PENTIUM_4) -DUSE_ICC -g -O3 #-Wall

STATLINK = -dn # Option to force static linkage

LIBS = $(USERLIBS) 
CFLAGS =  $(USE_GPROF) $(OPTIMIZE) $(USERFLAGS) 
OBJS = $(SRC:%=obj/%.o)

execs/$(TARGET).out: $(OBJS)
	$(LD) $(USE_GPROF) $(STATLINK) $(USERFLAGS) -o execs/$(TARGET) $(OBJS) $(LIBS)

$(OBJS): $(HEADERS:%=src/%.h) $(SRC:%=src/%.c)
	$(CC) -o $@ $(CFLAGS) -c $(*:obj/%=src/%.c)

clean:
	rm -f obj/*.o execs/$(TARGET)


