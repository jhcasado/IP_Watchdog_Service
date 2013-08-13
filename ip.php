<?php

$post = "cliente=" . $_POST['cliente'] . "&sucursal=" . $_POST['cliente'];


curl = curl_ini();
      if(curl) {
        curl_setopt(curl, CURLOPT_URL, "http://XXX.XXX.XXX.XXX:8081/cgi-vel/incidencias/capturar.pro");
        curl_setopt(curl, CURLOPT_POSTFIELDS, $post);
        res = curl_exec(curl);
        curl_easy_cleanup(curl);
      }


?> 