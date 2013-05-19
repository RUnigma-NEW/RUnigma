#-------------------------------
#---Update versions from vcs----
#---like git, svn---------------
#-------------------------------

# echo targets to launch
vcs-update-list:
	echo $(LIST_AUTOPKGV) |perl -pe 's/ /\n/g'

# remove and remake version .deps
vcs-update: $(LIST_AUTOPKGV)
	rm -f $^
	make $^