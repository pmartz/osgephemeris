<?PHP


    $download_dir = "Download/Generated/";
    $dir = opendir( $download_dir );
    $osgEphemeris = array();

    while( false !== ($file = readdir($dir)))
    {
        if( $file == "." || $file == ".." )
            continue;

        $a = explode( '-', $file );
        $product = $a[0];
        $b = explode('.', $a[1]);
        $version = $b[0];
        $bds = $b[0];

        array_push( $osgEphemeris, 
                    array(
                    'product' => $product, 
                    'version' => $version, 
                    'year'    => substr($bds, 0, 4), 
                    'month'   => substr($bds, 4, 2), 
                    'day'     => substr($bds, 6, 2),
                    'hour'    => substr($bds, 8, 2),
                    'minute'  => substr($bds, 10, 2) ,
                    'filename' => $file 
                        )
                  ); 
    }

    arsort( $osgEphemeris );
?>

<a href="tmp.php">_</a>

<?

    function print_row( $a )
    {
        global $download_dir;
        //$full_name = "Download/Generated/" . $a['filename'];
        $full_name = $download_dir . $a['filename'];
    ?>

        <tr align=center>
            <td> <?=$a['version']?> </td>
            <td> <?  
                $year=$a['year'];
                $month=$a['month'];
                $day=$a['day'];
                echo date( "d-M-Y", mktime( 0, 0, 0, $month, $day, $year ));  ?>

                    &nbsp;/&nbsp;

                    <?
                $hour=$a['hour'];
                $minute=$a['minute'];
                echo date( "H:i", mktime( $hour, $minute )); ?> 
            </td>


            <td align=left > 
            <a href=<?=$full_name?> ><?=$a['filename']?></a>
            </td>

            <td> <? 
                 $s = filesize($full_name)/1024; 
                $s = round($s);
                echo $s . "K";
                ?> </td>

            <td>
                <a href=<?=$full_name?> ><input type=button value="Download"></a>
            </td>
            <td>
                <form method=post action=delete.php>
                <input type="hidden" name="filename" value="Download/Generated/<?=$a['filename']?>">
                <input type="image" src="trash.gif" width=24 height=24>
                </form>
            </td>
        <tr>

    <?}
    ?>

<html>
<head>
  <title> osgEphemeris tarballs </title>
</head>

<body>
<center>
<table border=1 cellpadding=4 cellspacing=0 frame=box>
    <th bgcolor=#AABBCC colspan=6>osgEphemeris Generated Tarballs</th>

    <tr align=center bgcolor=#AABBCC>
        <td width=100>Version </td>
        <td width=150>Snapshot Date / Time
        <td> Tarball Name</td>
        <td> Size </td>
        <td width=100>&nbsp;</td>
        <td>&nbsp;</td>
    </tr>

    <tr>
    <th colspan=6 bgcolor=#DDDDDD align=left>

      <form method=post action=generate.php>
        <input type="submit" name="submit"     value="Generate">
      </form>

    <th colspan=6 bgcolor=#DDDDDD align=left> </th>
    </tr>

    <? array_walk( $osgEphemeris, "print_row" ); ?>

</table>
<p>
<center> <a href="<?=$_SERVER['HTTP_REFERER']?>"><input type=button value="&lt;- Back"></a> </center>
</body>

