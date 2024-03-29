<html>
    <head>
        <title>Weather Station</title>
	<meta http-equiv="refresh" content="10">
        <style type="text/css">
            body {
                background-color: #FFFFFF;
            }
            table {
                /* border-collapse: collapse;
                width: 100%;
                font-family: monospace;
                color: #eb4034;
                font-size : 30px;
                text-align: left;
                margin-bottom: 20px; */
                border-collapse: collapse;
                margin: 25px auto;
                font-size: 0.9em;
                font-family: sans-serif;
                min-width: 400px;
                box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);
            }
            table thead tr {
                background-color: #ff7200;
                color: #ffffff;
                text-align: left;
            }
            table th,
            table td {
                padding: 12px 15px;
            }

            table tbody tr {
                border-bottom: 1px solid #dddddd;
            }

            table tbody tr:nth-of-type(even) {
                background-color: #f3f3f3;
            }

            table tbody tr:last-of-type {
                border-bottom: 2px solid #ff7200;
            }

            table tbody tr.active-row {
                font-weight: bold;
                color: #ff7200;
            }

            /* th {
                background-color: #eb4034;
                color: white;
            } */

            tr:nth-child(even) {color: #ff7200; font-weight: bold;}
        </style>
    </head>
    <body>
            <?php
            $servername = "localhost";
            $username = "zero";
            $password = "12345678";
            $dbname = "wstation";

            // Create connection
            $conn = new mysqli($servername, $username, $password, $dbname);
            // Check connection
            if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
            }

            $sql = "SELECT id, room, device, dhth, dhtt,
                         bmpt, bmpp, timestamp FROM data ORDER BY device, timestamp DESC";
            $result = $conn->query($sql);

            $return = array();

            foreach($result as $val) {
                $return[$val["device"]][] = $val;
            }
            foreach ($return as $key => $value) {
                echo "<center><h2>$key</h2></center>";
                echo "<table>";
                echo "<thead>";
                echo "<th>Soba</th>";
                echo "<th>DHTH</th>";
                echo "<th>DHTT</th>";
                echo "<th>BMPT</th>";
                echo "<th>BMPP</th>";
                echo "<th>Vreme</th>";
                echo "</thead>";
                echo "<tbody>";

                $datat = array_slice($value, 0, 10);

                foreach ($datat as $keyt => $row) {
                    $date = new DateTime($row["timestamp"]);
                    $date = $date->format('d-m-Y H:i:s');
                    echo "<tr><td>" . $row["room"]. "</td><td>"
                                    . number_format($row["dhth"], 2)."%". "</td><td>"
                                    . number_format($row["dhtt"], 2)."°C". "</td><td>"
                                    . number_format($row["bmpt"], 2)."°C". "</td><td>"
                                    . number_format($row["bmpp"], 2, ".", "")."hPa". "</td><td>"
                                    . $date. "</td></tr>";
                }
                echo "</tbody></table><br>";
            }
            $conn->close();
            ?>
    </body>
</html>
