AC_DEFUN([TDT_CONDITIONAL],[
AC_SUBST([$1])dnl
if $2; then
  $1='yes'
else
  $1=''
fi
])