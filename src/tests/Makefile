include ModuleConfig
	ifeq ($(UseDoolpPreproc),yes)
		DOOLPPREPROC_LOG=doolpCC.__doolp.log
		ifndef DOOLPPREPROC_CPP
			DOOLPPREPROC_CPP = doolpCC.__doolp.cpp
		endif
		DOOLPPREPROC_O = $(DOOLPPREPROC_CPP:.cpp=.o)
		ifndef DOOLPPREPROC_HEADERS
			DOOLPPREPROC_HEADERS = $(DOOLPPREPROC_EXTRAHEADERS) $(wildcard *.h)
		endif
		ifndef DOOLPPREPROC_NOCPPS
			DOOLPPREPROC_CPPS = $(SOURCES_CPP)
		endif
		ifndef DOOLPPREPROC_XML
			DOOLPPREPROC_XML = "doolpCC.__doolp.xml"
		endif
	endif

	MODULELINKFLAGS_LIBS = $(foreach lib,$(MODULEUSESLIBS),-l$(lib) )
	MODULELINKFLAGS_LIBS_STATIC = $(foreach lib,$(MODULEUSESLIBS),$(shell find ../lib$(lib) -name "*.o"))

	SOURCES = $(wildcard *.c *.cpp)
	SOURCES_C = $(wildcard *.c)
	SOURCES_CPP = $(subst $(DOOLPPREPROC_CPP),,$(wildcard *.cpp))
	SOURCES_CPP += $(foreach dir, $(MODULESUBS), $(wildcard $(dir)/*.cpp))

	HEADERS = $(wildcard *.h)
	OBJECTS = $(SOURCES_C:.c=.o) $(SOURCES_CPP:.cpp=.o)
	OBJECTS += $(DOOLPPREPROC_O)


#	ifeq ($(PLATFORM),LINUX)
	MODULELINKFLAGS += -ldl -lpthread
#	else
#		MODULECOMPILEFLAGS += -D__DOOLP_STATIC_PTHREAD_MUTEX_INITIALIZER
#	endif 

#	MODULECOMPILEFLAGS += -D__$(PLATFORM)
	MODULECOMPILEFLAGS += -D__GLOG__MODULE__=$(MOD)
	EXTOBJS = $(foreach dir, $(MODULENEEDS), $(wildcard $(dir)/*.o))
	DEBUG_LIBEFENCE =  ../libefence.a # -lefence -L../ # ../libefence.a
# 	MODULELINKFLAGS += $(DEBUG_LIBEFENCE)
#	MODULELINKFLAGS += -lmallocdebug
#	MODULELINKFLAGS += /usr/lib/libdmalloccxx.a
all: .depends $(DEFAULTACTION)
	@echo All $(MOD) : Ok.

dep: cleanDepends makeDepends

cleanDepends:
	@echo Using Libs $(MODULEUSESLIBS)
	@echo MODULELINKFLAGS $(MODULELINKFLAGS)
	@rm -f .depends

.depends makeDepends:
	@echo Building Depends
	@$(foreach source, $(SOURCES_CPP), echo $(source) ; $(CC) -M -MT $(source:.cpp=.o) $(CFLAGS) $(MODULECOMPILEFLAGS) $(MODULECOMPILES) $(source) >> .depends ; )
include .depends

# Doolp Specific
doolpclean:
	rm -f *.__doolp.cpp

$(DOOLPPREPROC_CPP): $(DOOLPPREPROC_HEADERS) $(DOOLPPREPROC) ../doolpCC/generateCPP.xsl 
	@echo DoolpPreproc : building to $(DOOLPPREPROC_CPP)
	$(DOOLPPREPROC) $(DOOLPPREPROC_FLAGS) --headers $(DOOLPPREPROC_HEADERS) --cpp $(DOOLPPREPROC_CPPS) > $(DOOLPPREPROC_CPP) 2> $(DOOLPPREPROC_LOG)
#	xalan -IN $(DOOLPPREPROC_XML) -XSL ../doolpCC/generateCPP.xsl -OUT $(DOOLPPREPROC_CPP)


needs:
	@echo Checking Needs : $(MODULENEEDS)
	echo Compiling to : $(MOD)
# $(CC) $(CFLAGS) $(MODULECOMPILEFLAGS) $(MODULELINKFLAGS) $(MODULELINKFLAGS_LIBS) -o $(MOD) $(OBJECTS) $(EXTOBJS)
	@$(foreach dir, $(MODULENEEDS), pwd && cd $(dir) && $(MAKE) compile ; )

link: compile needs
	@echo Compiling to : $(MOD)
	$(CC) $(CFLAGS) $(MODULECOMPILEFLAGS) $(MODULELINKFLAGS) $(MODULELINKFLAGS_LIBS) -o $(MOD) $(OBJECTS) $(EXTOBJS)

linkstatic: compile needs
	echo Compiling static to : $(MOD) Static libraries "$(MODULELINKFLAGS_LIBS_STATIC)"
	$(CC) $(CFLAGS) $(MODULECOMPILEFLAGS) $(MODULELINKFLAGS) $(MODULELINKFLAGS_LIBS_STATIC) -o $(MOD) $(OBJECTS) $(EXTOBJS)


lib: compile needs
	echo Compiling to : $(MODULELIBNAME)
	$(CC) $(CFLAGS) $(MODULECOMPILEFLAGS) $(MODULELINKFLAGS) -shared -o $(MODULELIBNAME) $(OBJECTS) $(EXTOBJS)
#	$(CC) $(CFLAGS) $(MODULECOMPILEFLAGS) $(MODULELINKFLAGS) -shared -o $(MODULELIBNAME).a $(OBJECTS) $(EXTOBJS)

clean:
	@echo Cleaning...
	@rm -f $(OBJECTS) *~ *.a *.so *stackdump *.__doolp.cpp err
	@rm -f *.o
	@rm -f $(MOD)__UT $(MOD)
	@rm -f $(MOD).exe
	@rm -f .depends

cleanall: clean
	@echo Cleaning NEEDS
	@$(foreach dir, $(MODULENEEDS), cd $(dir) && $(MAKE) clean ; )
	@echo Cleaning SUBS
	@$(foreach dir, $(MODULESUBS), cd $(dir) && $(MAKE) clean ; )
	@echo Cleaning DEPENDS
	@$(foreach dir, $(MODULEDEPENDS), cd $(dir) && $(MAKE) clean ; )


compile: $(OBJECTS) # $(HEADERS) # $(SOURCES) $(HEADERS)
	@echo Compiled $(MOD) Objects : $(OBJECTS)

UT-compile: UT/$(MOD)__UT.o
	$(CC) $(CFLAGS) $(LINKFLAGS) -o UT/$(MOD)__UT UT/$(MOD)__UT.o $(filter-out $(MODULE_MAIN).o,$(OBJECTS)) $(EXTOBJS) $(MODULECOMPILEFLAGS) $(MODULELINKFLAGS)


UT: compile needs UT-compile 
	./UT/$(MOD)__UT

gdbUT: compile needs UT-compile
	gdb ./UT/$(MOD)__UT

gdb: compile link
	gdb ./$(MOD)

run: link
	./$(MOD) $(MODULERUNPARAMS)

runExtern:
	$(MODULERUNEXTERN)

install: link
	$(SUDO) cp -v $(MOD) $(MODULEINSTALLDIR)

libinstall: lib
	$(SUDO) cp -v $(MODULELIBNAME) $(MODULEINSTALLDIR_LIB)

.c.o: $< $(HEADERS)
	$(CC) -c $(CFLAGS) $(MODULECOMPILEFLAGS) -D__PURE_C__ -o $@ $<

.cpp.o: $< $(HEADERS)
	$(CC) -c $(CFLAGS) $(MODULECOMPILEFLAGS) -o $@ $<
