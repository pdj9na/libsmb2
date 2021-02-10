noinst_PROGRAMS = smb2-cat-async smb2-cat-sync \
	smb2-dcerpc-coder-test \
	smb2-ftruncate-sync \
	smb2-ls-async smb2-ls-sync \
	smb2-put-async \
	smb2-put-sync \
	smb2-raw-fsstat-async \
	smb2-raw-getsd-async \
	smb2-raw-stat-async \
	smb2-readlink \
	smb2-lsa-lookupsids \
	smb2-lseek-sync \
	smb2-share-enum \
	smb2-share-info \
	smb2-stat-sync \
	smb2-statvfs-sync \
	smb2-truncate-sync

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	"-D_U_=__attribute__((unused))" \
	$(WARN_CFLAGS)

COMMON_LIBS = ../lib/libsmb2.la
LDADD = $(COMMON_LIBS)
