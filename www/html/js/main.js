function hide() {
  document.getElementById("dform").style.visibility = "hidden";
}

function schema() {}

function show(sid) {
  fetch("http://127.0.0.1:8000/station/" + sid)
    .then((r) => r.json())
    .then((o) => {
      const x = o.timeseries.datetime;
      const y = o.timeseries.battery;
      const len = x.length;
      const xy = new Array(len);
      for (let i = 0; i < len; i++) xy[i] = [new Date(x[i]), y[i]];
      const div = document.getElementById("health");
      const g = document.createElement("div");
      div.appendChild(g);
      new Dygraph(g, xy, { width: "auto", labels: ["Date", "V"] });
    });
  document.getElementById("dform").style.visibility = "visible";
  return false;
}

function init_map() {
  let map = L.map("map", { minZoom: 3, zoomControl: false });
  L.tileLayer(
    "https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}",
    {
      attribution:
        "Tiles &copy; Esri &mdash; Esri, DeLorme, NAVTEQ, TomTom, Intermap, iPC, USGS, FAO, NPS, NRCAN, GeoBase, Kadaster NL, Ordnance Survey, Esri Japan, METI, Esri China (Hong Kong), and the GIS User Community",
    }
  ).addTo(map);
  L.control.scale().addTo(map);
  return map;
}

function search_item(e) {
  const li = document.createElement("li");
  const a = document.createElement("a");
  a.href = "#";
  let s = e.name;
  if (s !== "") s += ", ";
  li.innerHTML = s + "<span class='addendum'>" + e.id + "</span>";
  a.appendChild(li);
  return a;
}

fetch("http://127.0.0.1:8000/station")
  .then((r) => r.json())
  .then((l) => {
    var map = init_map();
    let lat = 0;
    let lng = 0;
    const ul = document.getElementById("search-results");
    for (let i = 0; i < l.length; i++) {
      const a = search_item(l[i]);
      a.addEventListener(
        "click",
        (ev) => {
          show(l[i].id);
          map.setView([l[i].lat, l[i].lng], 12);
        },
        false
      );
      ul.appendChild(a);
      L.marker([l[i].lat, l[i].lng])
        .addTo(map)
        .on("click", (ev) => show(l[i].id))
        .bindTooltip(a.lastChild.innerHTML);
      lat += l[i].lat;
      lng += l[i].lng;
    }
    lat /= l.length;
    lng /= l.length;
    map.setView([lat, lng], 9);
  });
