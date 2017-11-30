#!/bin/bash

if [ "$3" == "" ] || [ "$4" == "" ]
then
	echo "Usage vm-file.sh (get|put) src tgt"
	exit 1
fi

source "$1"

CMD="$2"

if [ "$CMD" == "put" ]
then
	SRC="$3"
	TGT="$VM_FTP_TDIR/$4"
else
	SRC="$VM_FTP_TDIR/$3"
	TGT="$4"
fi

cat >/tmp/batch.sftp <<EOF
"$2" "$3" "$4"
EOF

echo "$CMD" "$SRC" "$TGT" | sftp -P $VM_PORT localhost  

exit ${PIPESTATUS[1]}
