// get timestamp value
function timestamp() {
  let currentDate = new Date();
  let time = currentDate.getHours() + ":" + currentDate.getMinutes() + ":" + currentDate.getSeconds();
  return time;
}
// print in consola actiunile utilizatorului
function update_consola(id_parametru, parametru) {
  document.getElementById("consola").value += timestamp() + " ---> " + id_parametru + " = " + parametru + "\n";
  document.getElementById("consola").scrollTop = document.getElementById("consola").scrollHeight; //autoscroll
}
// convert form decimal to hexadecimal
function decToHex(num) {
  return num.toString(16)
}
//not in use; for future development to print transaction status
function afisare_raspuns_mb() {
  let parametru = decToHex(Number(document.getElementById("transaction_code").value));
  console.log(parametru);
  document.getElementById("consola").value += timestamp() + " ---> " + "Request result: " + parametru + "\n";
  document.getElementById("consola").scrollTop = document.getElementById("consola").scrollHeight; //autoscroll
}
// print in console modbus response value
function afisare_valoare_modbus() {
  let parametru = document.getElementById("mb_response").value;
  document.getElementById("consola").value += timestamp() + " ---> " + "Request response: " + parametru + "\n";
  document.getElementById("consola").scrollTop = document.getElementById("consola").scrollHeight; //autoscroll
}
// submit form 
function submit_value(id) {
  document.getElementById(id).submit();
}
// functie utilizata pentru a actualiza valoarea de start pentru multiple reads/writes
// functia preia valoarea de start adress de la celalalt form
function update_startAdressBit() {
  document.getElementById("startAdressBitCount").value = document.getElementById("startAdressBit").value
}
function update_startAdressCoil() {
  document.getElementById("startAdressCoilCount").value = document.getElementById("startAdressCoil").value
}

function AutoRefresh(t) {
  setTimeout("location.reload(true);", t);
}

/* Create buttons to open specific tab content. 
All <div> elements with class="tabcontent" are hidden by default (with CSS & JS). 
When the user clicks on a button - it will open the tab content that "matches" this button. */
function openTab(evt, tabName) {
  var i, tabcontent, tablinks;
  // Get all elements with class="tabcontent" and hide them
  tabcontent = document.getElementsByClassName("tabcontent");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }
  // Get all elements with class="tablinks" and remove the class "active"
  tablinks = document.getElementsByClassName("tablinks");
  for (i = 0; i < tablinks.length; i++) {
    tablinks[i].className = tablinks[i].className.replace(" active", "");
  }
  // Show the current tab, and add an "active" class to the button that opened the tab
  document.getElementById(tabName).style.display = "block";
  evt.currentTarget.className += " active";
}
// Get the element with id="defaultOpen" and click on it
function default_open() {
  document.getElementById("defaultOpen").click();
}

// verificare continut mesaj 
function verificare_mesaj() {
  if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('open', function (e) {
      console.log("Events Connected");
    }, false);

    source.addEventListener('error', function (e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      }
    }, false);

    source.addEventListener('message', function (e) {
      console.log("message", e.data);
    }, false);

    source.addEventListener('voltage', function (e) {
      console.log("voltage", e.data);
      document.getElementById("batteryVoltage").value = e.data;
    }, false);

    // source.addEventListener('transaction_code', function (e) {
    //   console.log("transaction_code", e.data);
    //   document.getElementById("transaction_code").value = e.data;
    //   update_consola_raspuns(e.data);
    // }, false);

    source.addEventListener('mb_response', function (e) {
      console.log("mb_response", e.data);
      document.getElementById("mb_response").value = e.data;
      update_consola_raspuns(e.data);
    }, false);

  }

}

