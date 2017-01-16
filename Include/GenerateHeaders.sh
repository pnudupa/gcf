# Create the GCF headers from scratch
rm -fr GCF3 2> /dev/null
mkdir GCF3

cd ../Core
for a in `ls *.h`
do
    echo '#include "../../Core/'$a'"' > ../Include/GCF3/`echo $a | cut -f1 -d.`
done

cd ../Gui
for a in `ls *.h`
do
    echo '#include "../../Gui/'$a'"' > ../Include/GCF3/`echo $a | cut -f1 -d.`
done

cd ../Quick
for a in `ls *.h`
do
    echo '#include "../../Quick/'$a'"' > ../Include/GCF3/`echo $a | cut -f1 -d.`
done

cd ../Ipc
for a in `ls *.h`
do
    echo '#include "../../Ipc/'$a'"' > ../Include/GCF3/`echo $a | cut -f1 -d.`
done

cd ../GDrive/Lite
for a in `ls I*.h`
do
    echo '#include "../../GDrive/Lite/'$a'"' > ../../Include/GCF3/`echo $a | cut -f1 -d.`
done
cd ..

cd ../Include/GCF3
rm -fr *_p
rm -fr GuiCommon
rm -fr QuickCommon
rm -fr IpcCommon
cd ..
