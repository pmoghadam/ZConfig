<?php /* Public domain */ ?>

<form method=post action=''>
Hostname: <input type="text" name="hostname" />
MAC: <input type="text" name="mac" />
IP: <input type="text" name="ip" />
<input type="submit" name="action" value="add" />
<input type="submit" name="action" value="del" />
</form>
<br>

<?php
if( isset($_POST["hostname"]) && 
    isset($_POST["mac"])      && 
    isset($_POST["ip"])       &&
    isset($_POST["action"])   )
{
    $hostname = $_POST["hostname"];
    $mac = $_POST["mac"];
    $ip = $_POST["ip"];
    $action = $_POST["action"];

    $snd_msg = "dhcpd-static-lease.sh $action $hostname $mac $ip";
    $rcv_msg = "";
    $timeout = 10;
    $socket = stream_socket_client('unix:///usr/local/zconfig/zconfig.sock', 
                                    $errorno, $errorstr, $timeout);
    stream_set_timeout($socket, $timeout);

    if(!fwrite($socket, $snd_msg))
            echo("Error while sending message !!!<br>\n");

    if (!($rcv_msg = fread($socket, 1024))) 
            echo("Error while receiving message!!!<br>\n");
    else 
            echo($rcv_msg."<br>\n");
}
?>
