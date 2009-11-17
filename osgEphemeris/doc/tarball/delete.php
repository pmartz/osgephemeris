<?  
    $filename=$_POST['filename'];
    if( $filename != NULL )
    {
        unlink( $filename );
    }
?>

<html>
<META http-equiv="Refresh" content="0;url=index.php">
</html>


