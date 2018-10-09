s="[Desktop Entry]\nName=海天鹰Markdown编辑器\nComment=一个极简的Markdown编辑器\nExec=`pwd`/HTYMD %u\nIcon=`pwd`/icon.svg\nPath=`pwd`\nTerminal=false\nType=Application\nMimeType=text/markdown;\nCategories=TextEditor;"
echo -e $s > HTYMD.desktop
cp `pwd`/HTYMD.desktop ~/.local/share/applications/HTYMD.desktop