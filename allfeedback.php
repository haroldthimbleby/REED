<!DOCTYPE html>
<html>
   	<head>
		<style>
			body { font-family: sans-serif; }
		</style>
		<title>All REED feedback</title>
	</head>
	<body>
        <?php
        	$feedbackFileName = "feedback.txt";
        	echo "<h1>All REED feedback</h1>\n".file_get_contents($feedbackFileName, FILE_USE_INCLUDE_PATH)."\n";  
        ?>
    </body>
</html>
