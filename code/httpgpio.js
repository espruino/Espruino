

function onPageRequest(req, res) {
  res.writeHead(200, {'Content-Type': 'text/html'});
  res.write('<html><body>');
  res.write('<p>Pin is '+(D25.read()?'on':'off')+'</p>');
  res.write('<a href="/on">on</a><br/><a href="/off">off</a>');
  res.end('</body></html>');
  if (req.url=="/on") digitalWrite(D7, 1);
  if (req.url=="/off") digitalWrite(D7, 0);
}
http.createServer(onPageRequest).listen(8080);

