#  (C) 2001-2010 Force Dimension
#  All Rights Reserved.


SUBDIRS = gravity \
					encoders \
					torques \
					reflections \
					multireflections \
					graspreflections

all: $(SUBDIRS)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

clean:
	@for T in $(SUBDIRS); do make -C $$T $@; done
	-rm -f core *~ *.bak #*
