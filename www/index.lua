function td(content)
   return "<td>" .. content .. "</td>"
end


function tr(...)
   local ret = ""
   
   for i,s in ipairs(arg) do
      ret = ret .. s
   end
   
   return "<tr>" .. ret .. "</tr>\n"
end

function sensor_to_tr(id,sensor)
   return tr(td(sensor["name"]),
	     td(sensor["measure_last"]),
	     td(sensor["measure_min"]),
	     td(sensor["measure_max"]))
end

str = "<html><head><link rel='stylesheet' type='text/css' href='/style.css' /></head><body><h1>Psensor Monitoring Server</h1>"

if cpu and cpu["load"] then
   str = str .. "<h2>System</h2>"
      .. "<table>"
      .. "<tr class='title'><th>Name</th><th>Value</th></tr>"
      .. "<tr><td>CPU Usage</td><td>" .. math.ceil(100*cpu["load"]) .. "%</td></tr>"
   .. "</table>"
				   
end

str = str .. "<h2>Sensors</h2>"
   .. "<table>"
   .. "<tr class='title'><th>Name</th><th>Value</th><th>Min</th><th>Max</th></tr>"

for i,sensor in ipairs(sensors) do 
   str = str .. sensor_to_tr(i,sensor) 
end

str = str .. "</table><hr /><a href='http://wpitchoune.net/psensor'>psensor-server</a></body></html>"

return str

