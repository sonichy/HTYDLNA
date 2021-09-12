s="[Desktop Entry]\nName=海天鹰DLNA\nComment=A mini DLNA sender\nExec=`pwd`/HTYDLNA\nIcon=`pwd`/HTYDLNA.png\nPath=`pwd`\nTerminal=false\nType=Application\nCategories=AudioVideo;"
echo -e $s > HTYDLNA.desktop
cp `pwd`/HTYDLNA.desktop ~/.local/share/applications/HTYDLNA.desktop