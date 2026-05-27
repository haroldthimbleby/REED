<html>
    <body>
        <?php
        	$feedbackFileName = "feedback.txt";
        	echo "<html><body><h1>REED feedback</h1>".file_get_contents($feedbackFileName, FILE_USE_INCLUDE_PATH)."</body></html>";  
        ?>
    </body>
</html>
