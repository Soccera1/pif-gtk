#
# Regular cron jobs for the pif-gtk package.
#
0 4	* * *	root	[ -x /usr/bin/pif-gtk_maintenance ] && /usr/bin/pif-gtk_maintenance
