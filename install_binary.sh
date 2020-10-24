#!/bin/bash

if [[ $EUID > 0 ]]; then
  # echo ""
  echo "Non-root user detected. If you have issues running this script, try running as root."
fi

echo ""

echo "Installing procon_driver to /usr/local/bin"
install build/procon_driver /usr/local/bin


retVal=$?
echo ""
if [ $retVal -ne 0 ]; then
  echo "Error."
  echo "You could try again as root (sudo ./install_binary.sh)."
else
  echo "Success."
fi
exit $retVal
