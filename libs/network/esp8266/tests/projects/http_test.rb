# HTTP test responder: simple web server that responds to requests from espruino in order to
# test sockets

require 'sinatra'

get "/ping" do
  "pong\nEND"
end

get "/empty" do
  ""
end

get "/data" do
  size = params[:size].to_i || 4013
  text = "This is a #{size} byte response from the friendly test server\n"
  resp = text * (size / text.length)
  if resp.length < size
    resp += text[0, size-resp.length]
  end
  if size > 3
    resp[-3, 3] = "END"
  end
  resp
end

post "/data" do
  body = request.body.read
  body.length.to_s
end

post "/temp" do
  temp = params["temp"]
  if params["temp"]
    puts "Got temperature: #{params["temp"]}C"
    "OK"
  else
    halt 400
  end
end
