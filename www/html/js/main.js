function createConfig(meta, schema) {
  const dl = document.getElementById("sensors");
  dl.replaceChildren();
  for (const [k, v] of Object.entries(meta.config)) {
    const dt = document.createElement("dt");
    dt.style = "text-transform: none; font-weight: 500";
    dt.appendChild(document.createTextNode(k));

    const dd = document.createElement("dd");

    let label = document.createElement("label");
    label.appendChild(document.createTextNode("Sensor:"));
    const sel = document.createElement("select");
    sel.name = k;
    let opt = document.createElement("option");
    opt.value = -1;
    opt.appendChild(document.createTextNode("Pending configuration"));
    sel.appendChild(opt);
    for (let i = 0; i < schema.length; i++) {
      opt = document.createElement("option");
      opt.value = i;
      opt.appendChild(document.createTextNode(schema[i].name));
      sel.appendChild(opt);
    }
    sel.selectedIndex = meta.config[k].sensor + 1;
    label.appendChild(sel);

    dd.appendChild(label);

    label = document.createElement("label");
    label.appendChild(document.createTextNode("Label:"));
    const input = document.createElement("input");
    input.value = meta.config[k].label;
    input.name = k;
    label.append(input);

    dd.appendChild(label);

    dl.appendChild(dt);
    dl.appendChild(dd);
  }
}

function createGraphs(sid, c, time, series) {
  c.replaceChildren();
  for (const [k, s] of Object.entries(series)) {
    const div = document.createElement("div");
    div.style = "height: 400px";
    c.appendChild(div);
    if (s == null) {
      div.innerHTML = `<div class="alert"><h2>Sensor ${k} is not configured yet</h2></div>`;
      continue;
    }
    opts = {
      title: s.title,
      xlabel: "Date",
      ylabel: s.labels[0],
      width: "auto",
      labels: ["Date"].concat(s.labels),
      labelsSeparateLines: true,
      series: {},
      axes: {
        y: { axisLabelWidth: 72 },
        y2: { axisLabelWidth: 72 },
      },
    };
    if (s.length > 1) {
      opts["y2label"] = s.labels[1];
      opts.series[s.labels[0]] = { color: "#47a" };
      opts.series[s.labels[1]] = { color: "#e67", axis: "y2" };
    } else {
      opts.series[s.labels[0]] = { color: "#283" };
    }
    const g = new Dygraph(
      div,
      time.map((e, i) => {
        return [new Date(e * 1000)].concat(s.data[i]);
      }),
      opts,
    );
    const b = document.createElement("button");
    b.addEventListener("click", (e) => {
      location.href = `http://127.0.0.1:8000/station/${sid}/${k}/download`;
    });
    b.classList.add("btn");
    b.appendChild(document.createTextNode("Download"));
    const d = document.createElement("div");
    d.appendChild(b);
    c.appendChild(d);
  }
}

function hide() {
  document.getElementById("form").style.visibility = "hidden";
}

function show(sid) {
  fetch("http://127.0.0.1:8000/station/" + sid)
    .then((r) => r.json())
    .then(({ schema, meta, data }) => {
      document
        .getElementById("sid")
        .replaceChildren(document.createTextNode(meta.id));
      const form = document.getElementById("meta");
      for (const [k, v] of Object.entries(meta)) {
        const field = form.querySelector(`input[name="${k}"]`);
        field && (field.value = v);
      }
      createConfig(meta, schema);
      createGraphs(
        meta.id,
        document.getElementById("health"),
        data.time,
        data.health,
      );
      createGraphs(
        meta.id,
        document.getElementById("data"),
        data.time,
        data.series,
      );
    });
  document.getElementById("form").style.visibility = "visible";
  return false;
}

async function submit(form) {
  const o = {
    name: form.querySelector('input[name="name"]').value,
    lat: parseFloat(form.querySelector('input[name="lat"]').value || 0),
    lng: parseFloat(form.querySelector('input[name="lng"]').value || 0),
    note: form.querySelector('input[name="note"]').value,
  };
  const items = document.getElementById("sensors").children;
  const config = {};
  for (let i = 0; i < items.length; i += 2) {
    const k = items[i].childNodes[0].nodeValue;
    config[k] = {
      sensor: parseInt(
        items[i + 1].querySelector(`select[name="${k}"]`).value || -1,
      ),
      label: items[i + 1].querySelector(`input[name="${k}"]`).value,
    };
  }
  o["config"] = config;
  const sid = document.getElementById("sid").childNodes[0].nodeValue;
  await fetch(`http://127.0.0.1:8000/station/${sid}/update`, {
    method: "POST",
    headers: { Accept: "application/json", "Content-Type": "application/json" },
    body: JSON.stringify(o),
  });
}

function init_map() {
  const uri =
    "https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}";
  const copyright =
    "Tiles &copy; Esri &mdash; Esri, DeLorme, NAVTEQ, TomTom, Intermap, iPC, USGS, FAO, NPS, NRCAN, GeoBase, Kadaster NL, Ordnance Survey, Esri Japan, METI, Esri China (Hong Kong), and the GIS User Community";
  let map = L.map("map", { minZoom: 3, zoomControl: false });
  L.tileLayer(uri, { attribution: copyright }).addTo(map);
  L.control.scale().addTo(map);
  return map;
}

function search_item(e) {
  const li = document.createElement("li");
  const a = document.createElement("a");
  a.href = "#";
  let s = e.name;
  if (s !== "") s += ", ";
  li.innerHTML = `${s}<span class='addendum'>${e.id}</span>`;
  a.appendChild(li);
  return a;
}

const form = document.getElementById("meta");
form.addEventListener("submit", (e) => {
  e.preventDefault();
  submit(form);
  location.reload();
});

fetch("http://127.0.0.1:8000/station")
  .then((r) => r.json())
  .then((l) => {
    var map = init_map();
    let lat = 0;
    let lng = 0;
    const ul = document.getElementById("sbar-list");
    for (let i = 0; i < l.length; i++) {
      const latlng = [l[i].lat, l[i].lng];
      const a = search_item(l[i]);
      a.style = "text-overflow: ellipses";
      a.addEventListener(
        "click",
        (ev) => {
          show(l[i].id);
          map.setView(latlng, 12);
        },
        false,
      );
      ul.appendChild(a);
      L.marker(latlng)
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
