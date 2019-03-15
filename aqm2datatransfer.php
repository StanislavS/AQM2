<?php
header('Access-Control-Allow-Origin: *');
header("Content-type: text/html; charset=utf-8");

$filename = getcwd().'/bin/'.$_GET['device'].'.txt';
$codline = ""; 
$codline = $codline."password".$_GET['password'].PHP_EOL;
$codline = $codline."hour=".$_GET['hour'].PHP_EOL;                 //*** record time
$codline = $codline."minute=".$_GET['minute'].PHP_EOL;               //*** record time
$codline = $codline."second=".$_GET['second'].PHP_EOL;               //*** record time
$codline = $codline."day=".$_GET['day'].PHP_EOL;                  //*** record date
$codline = $codline."month=".$_GET['month'].PHP_EOL;                //*** record date
$codline = $codline."year=".$_GET['year'].PHP_EOL;                 //*** record date
$codline = $codline."longtitude=".$_GET['longtitude'].PHP_EOL;             //*** GPS longtitude
$codline = $codline."lattitude=".$_GET['lattitude'].PHP_EOL;              //*** GPS lattitude
$codline = $codline."altitude=".$_GET['altitude'].PHP_EOL;             //*** GPS altitude
$codline = $codline."temp=".$_GET['temp'].PHP_EOL;                 //*** Temperature C*           
$codline = $codline."humi=".$_GET['humi'].PHP_EOL;                 //*** Humidity %
$codline = $codline."press=".$_GET['press'].PHP_EOL;                //*** Pressure mm/H
$codline = $codline."pm1=".$_GET['pm1'].PHP_EOL;                 //*** Dust pm1.0 uG/m3
$codline = $codline."pm25=".$_GET['pm25'].PHP_EOL;                //*** Dust pm2.5 uG/m3
$codline = $codline."pm10=".$_GET['pm10'].PHP_EOL;                //*** Dust pm10.0 uG/m3
$codline = $codline."OSONE=".$_GET['OSONE'].PHP_EOL;                  //*** Ozon (O3), ppm 
$codline = $codline."SO2=".$_GET['SO2'].PHP_EOL;                    //*** Sulfur Dioxide (SO2), ppm 
$codline = $codline."NO2=".$_GET['NO2'].PHP_EOL;                    //*** Nitrogen Dioxide (NO2), ppm
$codline = $codline."CO=".$_GET['CO'].PHP_EOL;                     //*** Carbon Monoxide (CO), ppm
$codline = $codline."NH3=".$_GET['NH3'].PHP_EOL;                    //*** Ammonia  (NH3), ppm
$codline = $codline."CO2=".$_GET['CO2'].PHP_EOL;                 //*** Carbon Dioxide (CO2), ppm
$codline = $codline."RAD=".$_GET['RAD'].PHP_EOL;                 //*** Radioactive Background, uR/h  
$codline = $codline."SOUND=".$_GET['SOUND'].PHP_EOL;                  //*** Sound Level, dB

$fp = fopen($filename, "a"); // ("r" - считывать "w" - создавать "a" - добовлять к тексту),мы создаем файл
$str = date("Y-m-d H:i:s")." | ".$codline;

$bb = fwrite($fp, $str);

fclose($fp);
echo("1");


?>
