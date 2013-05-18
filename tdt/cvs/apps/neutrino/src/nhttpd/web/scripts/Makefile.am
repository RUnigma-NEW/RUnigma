install-data-local:
	install -d $(DATADIR)/neutrino/httpd/scripts
	install -m 0755 Y_*.sh $(DATADIR)/neutrino/httpd/scripts
	install -m 0644 _Y_*.sh $(DATADIR)/neutrino/httpd/scripts
	install -m 0755 api.sh $(DATADIR)/neutrino/httpd/scripts

uninstall-local:
	-rm -rf $(DATADIR)/neutrino/httpd/scripts
