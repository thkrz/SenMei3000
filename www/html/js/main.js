function toggleNav() {
	const e = document.getElementById("nav");
	if (e.style.width === "264px") e.style.width = 0;
	else e.style.width = "264px";
}

function createConfig(meta) {
	const tbl = document.getElementById("sensor-config");
	tbl.replaceChildren();
	for (const [k, v] of Object.entries(meta.config)) {
		const row = document.createElement("tr");
		row.name = k;
		row.innerHTML = `
      <td>
        Sensor&nbsp;<span style="text-transform: none">${k}</span>
      </td>
      <td>
        <input disabled value="${v.sensor}" />&nbsp;/
      </td>
      <td>
        <input name="label" value="${v.label}" placeholder="ENTER DESCRIPTION" />
      </td>`;
		tbl.appendChild(row);
	}
}

function createGraphs(sid, c, time, series) {
	c.replaceChildren();
	for (const [k, s] of Object.entries(series)) {
		const div = document.createElement("div");
		div.style = "height: 400px";
		c.appendChild(div);
		if (s.length === 0) {
			div.innerHTML = `<div class="alert">
        <span class="material-icons">warning_amber</span>
        <h2>${s.title}</h2>
        </div>`;
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
			opts.y2label = s.labels[1];
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
		const box = document.createElement("div");
		box.appendChild(b);
		c.appendChild(box);
	}
}

function hide() {
	document.getElementById("form").style.visibility = "hidden";
}

function show(sid) {
	fetch("http://127.0.0.1:8000/station/" + sid)
		.then((r) => r.json())
		.then(({ meta, data }) => {
			document
				.getElementById("sid")
				.replaceChildren(document.createTextNode(meta.id));
			const form = document.getElementById("meta");
			for (const [k, v] of Object.entries(meta)) {
				const field = form.querySelector(`input[name="${k}"]`);
				field && (field.value = v);
			}
			createConfig(meta);
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
		lat: Number.parseFloat(form.querySelector('input[name="lat"]').value || 0),
		lng: Number.parseFloat(form.querySelector('input[name="lng"]').value || 0),
	};
	const config = {};
	for (const el of document
		.getElementById("sensor-config")
		.getElementsByTagName("tr")) {
		const k = el.name;
		config[k] = {
			label: el.querySelector('input[name="label"]').value,
		};
	}
	o.config = config;
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
	const map = L.map("map", { minZoom: 3, zoomControl: false });
	L.tileLayer(uri, { attribution: copyright }).addTo(map);
	L.control.scale({ position: "bottomright" }).addTo(map);
	return map;
}

function search_item(e) {
	const li = document.createElement("li");
	const a = document.createElement("a");
	a.href = "javascript:void(0)";
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
		const map = init_map();
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
