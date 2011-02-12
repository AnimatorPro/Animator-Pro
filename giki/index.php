<?php

include_once "markdown.php"

//get a list of .markdown files
//get index.html
//get the name of the current "page", and look it up in the list of markdown
//files

//if it's not there, print an error message. 

//if it is there, get the markdown text, convert it to HTML
//and insert the html fragment into the "content" element
//of index.html

//get the index.markdown, convert to html, and put it into the nav element
//sort the list of markdown by date, and create html to insert into
//the history element.
//echo the html into the output.

$index_html = new DOMDocument();
$index_html->loadHTMLFile("index.html");



//$content_html = Markdown($my_text);


?>