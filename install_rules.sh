#!/bin/bash

echo "This script installs rules for supported controllers. Without those rules, the driver would always need root access to communicate with the controllers."

if [[ $EUID > 0 ]]; then
  echo ""
  echo "Non-root user detected. If you have issues running this script, try running as root."
fi

echo ""

#copy udev rules
echo "Installing rule: 71-nintendo-controllers.rules"
cp udev_rules/71-nintendo-controllers.rules /etc/udev/rules.d/71-nintendo-controllers.rules


retVal=$?
echo ""
if [ $retVal -ne 0 ]; then
  echo "Error."
  echo "You could try again as root (sudo ./install_rules.sh)"
  exit $retVal
fi

# Trigger a rules reload.
udevadm control --reload-rules && udevadm trigger


retVal=$?
echo ""
if [ $retVal -ne 0 ]; then
  echo "Error."
  echo "You could try again as root (sudo ./install_rules.sh)"
else
  echo "Successful."
fi
exit $retVal
