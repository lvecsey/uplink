
Keep an up vote counter in memory, based on the URL page that is submitted.

You can spawn the FastCGI process as follows:

```console
exec spawn-fcgi -p 8326 -n ./uplink
```

And then configure your nginx server or similar web server:

```
	      location /uplink {
	      include fastcgi_params;
	      fastcgi_pass 127.0.0.1:8326;
	      }
```	      


An AJAX example is provided in uplink_examplepage.html

change the my_url based on where you host the page. You should also change the locations of the uplink server, from example.com to your own website domain.

You will notice that reloading the page will simply retrieve the current counter value. Pressing the Uplink button will increment the counter (through a server request)

