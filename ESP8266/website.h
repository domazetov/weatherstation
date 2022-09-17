const char main_head[] = R"=====(
<!DOCTYPE html>
<html lang='en'>
   <head>
      <meta name='viewport' content='width=device-width'/>
      <title>Setup</title>
      <style> *, ::after, ::before { box-sizing: border-box; }
         body {
         margin: 0;
         font-family: 'Segoe UI', Roboto, 'Helvetica Neue', Arial, 'Noto Sans', 'Liberation Sans';
         font-size: 1rem;
         font-weight: 400;
         line-height: 1.5;
         color: #212529;
         background-color: #f5f5f5;}
         .form-control {
         display: block;
         width: 100%;
         height: calc(1.5em + 0.75rem + 2px);
         border: 1px solid #ced4da;}
         button {
         cursor: pointer;
         border: 1px solid transparent;
         color: #fff;
         background-color: #ff7200;
         border-color: #ff7200;
         padding: 0.5rem 1rem;
         font-size: 1.25rem;
         line-height: 1.5;
         border-radius: 0.3rem;
         width: 100%;}
         .form-signin {
         width: 100%;
         max-width: 400px;
         padding: 15px;
         margin: auto;}
         h1 { text-align: center; }
      </style>
   </head>
)=====";

#if ENABLE_IR
const char main_body[] = R"=====(
 <body>
      <main class='form-signin'>
      <form action='wifisave' method='post'>
         <h1>WiFi Setup</h1>
         <br />
         <div class='form-floating'>
            <label><b>SSID:</b></label>
            <input type='text' class='form-control' placeholder='network' name='n'/>
         </div>
         <div class='form-floating'>
            <br /><label><b>Password:</b></label>
            <input type='password' class='form-control' placeholder='password' name='p'/>
         </div>
         <div class='form-floating'>
            <br /><label><b>Broker:</b></label>
            <input type='ip-address' class='form-control' placeholder='ip-address'name='b'/>
         </div>
         <br /><button type='submit'>Save</button>
      </form>
      <br />
      <form method='get' action='acsave'>
         <button type='submit'>Setup AC</button>
      </form>
)=====";
#else
const char main_body[] = R"=====(
 <body>
      <main class='form-signin'>
      <form action='wifisave' method='post'>
         <h1>WiFi Setup</h1>
         <br />
         <div class='form-floating'>
            <label><b>SSID:</b></label>
            <input type='text' class='form-control' placeholder='network' name='n'/>
         </div>
         <div class='form-floating'>
            <br /><label><b>Password:</b></label>
            <input type='password' class='form-control' placeholder='password' name='p'/>
         </div>
         <div class='form-floating'>
            <br /><label><b>Broker:</b></label>
            <input type='ip-address' class='form-control' placeholder='ip-address'name='b'/>
         </div>
         <br /><button type='submit'>Save</button>
      </form>
      <br />
)=====";
#endif