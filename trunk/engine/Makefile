DEMO ?= 0

TOP_DIR = ..
MODULE_TOP_DIR = .
BUILD_NAME = libengine
SUB_MOD =
SUB_DIR = 
CFLAGS_LOCAL = -DENGINE_DEBUG=1 -Wno-sign-compare
ifeq ($(DEMO),1)
BUILD_TYPE = bin
BUILD_NAME = engine
CFLAGS_LOCAL += -DENGINE_DEBUG=1 -DENGINE_MAIN=1 -Wno-sign-compare
LDFLAGS_LOCAL = -lmemory -ldl
else
BUILD_TYPE = dynlib
endif

include $(TOP_DIR)/script/compile.mk

prepare_local:
	$(BISON) eng_rule.y
	$(FLEX) eng_rule.l
	$(FREPLACE) YYSTYPE ENG_RULE_STYPE eng_rule_lex.h
	$(FREPLACE) YYSTYPE ENG_RULE_STYPE eng_rule_lex.c
	$(FREPLACE) YYLTYPE ENG_RULE_LTYPE eng_rule_lex.h
	$(FREPLACE) YYLTYPE ENG_RULE_LTYPE eng_rule_lex.c
	$(FREPLACE) eng_rule__ eng_rule_ eng_rule_lex.h
	$(FREPLACE) eng_rule__ eng_rule_ eng_rule_lex.c

clean_local:
	$(RM) eng_rule_parser.* eng_rule_lex.* 
