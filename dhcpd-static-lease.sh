#!/bin/bash
# Public domain
#
set -e

# Configuration
CONF="/etc/dhcpd.conf"
AWK=$(mktemp)
TMP=$(mktemp)

# Syntax check
if [ "$#" -ne "4" ] || ( [ "$1" != "add" ] && [ "$1" != "del" ] ); then
	echo "Syntax: $0 [add|del] [Hostname] [MAC] [IP]"
	exit 1
fi

# Get command line options
ACTION="$1"
HOST="$2"
MAC="$3"
IP="$4"

# Create config line 
CFG_LINE="host $HOST { hardware ethernet $MAC; fixed-address $IP; }"


# Adding config line
if [ "$ACTION" = "add" ]; then

  # Duplication check
  if grep -q "host $HOST" $CONF; then
    echo "Error: Host $HOST exist in $CONF !!!"
    exit 1
  fi
  if grep -q "hardware ethernet $MAC" $CONF; then
    echo "Error: Hardware ethernet $MAC exist in $CONF !!!"
    exit 1
  fi
  if grep -q "fixed-address $IP" $CONF; then
    echo "Error: fixed-address $IP exist in $CONF !!!"
    exit 1
  fi

  # Create awk script
  cat > $AWK << EOF
    {
      if ( \$0 ~ "{" )
        count++
      if ( \$0 ~ "}" )
      {
        count--
        if ( count == 0 )
          print  "$CFG_LINE"
      }
      print
    }
EOF

  # Run awk script
  awk -f $AWK $CONF > $TMP
  cp $TMP $CONF

  echo "Successfully added: $CFG_LINE to $CONF"
fi

# Deleting config line
if [ "$ACTION" = "del" ]; then
  #sed -i -e "s,$CFG_LINE,," $CONF
  sed -i -e "/^$CFG_LINE$/d" $CONF
  echo "Successfully deleted: $CFG_LINE from $CONF"
fi

# Reload config file
killall dhcpd
sleep 1
dhcpd -q

# Cleanup
rm $AWK
rm $TMP

