

function onPageRequest(req, res) {
  res.writeHead(200, {'Content-Type': 'text/html'});
  res.write('<html><body>');
  res.write('<p>Pin is '+(BTN.read()?'on':'off')+'</p>');
  res.write('<a href="/on">on</a><br/><a href="/off">off</a>');
  res.end('</body></html>');
  if (req.url=="/on") digitalWrite(LED1, 1);
  if (req.url=="/off") digitalWrite(LED1, 0);
}
http.createServer(onPageRequest).listen(8080);

