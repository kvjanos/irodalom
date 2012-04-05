var menu_open = false;
var timeout_id = null;

function toggle_menu(menu_id)
{      
   if(menu_open) hide_menu(menu_id);
   else show_menu(menu_id);
}

function show_menu(menu_id)
{  
   if (timeout_id) timeout_id = window.clearTimeout(timeout_id); 
   var menu_id = document.getElementById(menu_id);
   menu_id.style.visibility = 'visible';
   menu_open = true;
}

function schedule_hide(menu_id)
{
   if (timeout_id) timeout_id = window.clearTimeout(timeout_id);
   timeout_id = window.setTimeout("hide_menu('" + menu_id + "')", 600);
}
   
function hide_menu(menu_id)
{
   var menu_id = document.getElementById(menu_id);
   menu_id.style.visibility = 'hidden';
   menu_open = false;
}

/*
var menu_open = false;

function toggle_menu(id)
{   
   var id = document.getElementById(id);
   
   if(menu_open) {
      hide_menu(id);
   }
   else {
      show_menu(id);
   }
   
   menu_open = menu_open ? false : true;
   
   //return false;   
}

function show_menu(id) { id.style.visibility = 'visible'; }

function hide_menu(id) { id.style.visibility = 'hidden'; }
*/
