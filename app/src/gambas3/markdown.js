function $(id)
{
  return document.getElementById(id);
}

function goto_line(line)
{
  var sel = [null, null, null];
  var r1, r2;
  var i, n;
  var elt = $('l' + line);
  var elt_next;
  
  if (!elt || elt == undefined)
    return;
  
  for (i = 0; i < sel.length; i++)
  {
    sel[i] = $('_sel' + i);
    if (!sel[i])
    {
      sel[i] = document.createElement('div');
      sel[i].id = '_sel' + i;
      sel[i].style.position = 'absolute';
      sel[i].style.backgroundColor = '#FFFF80';
      sel[i].style.opacity = '0.3';
      sel[i].style.pointerEvents = 'none';
      sel[i].style.zIndex = 1000;
      document.body.appendChild(sel[i]);
    }
    sel[i].style.display = 'none';
  }
  
  r1 = elt.getBoundingClientRect();
  
  r1.x += document.body.scrollLeft;
  r1.y += document.body.scrollTop;
  
  for (n = 0;; n++)
  {
    elt_next = $('l' + (line + n))
    if (elt_next)
    {
      r2 = elt_next.getBoundingClientRect();
      r2.x += document.body.scrollLeft;
      r2.y += document.body.scrollTop;
    }
    else
      r2 = {x: 0, y: document.body.offsetHeight, width: document.body.offsetWidth, height: 0};
    
    if (!elt_next || r2.y > r1.y)
      break;
  }
  
  sel[0].style.left = r1.x + 'px';
  sel[0].style.top = r1.y + 'px';
  sel[0].style.width = (document.body.offsetWidth - r1.x) + 'px';
  sel[0].style.height = r1.height + 'px';
  sel[0].style.display = 'block';
  
  sel[1].style.left = '0';
  sel[1].style.top = (r1.y + r1.height) + 'px';
  sel[1].style.width = '100%';
  sel[1].style.height = (r2.y - r1.y - r1.height) + 'px';
  sel[1].style.display = 'block';
  
  sel[2].style.left = '0';
  sel[2].style.top = r2.y + 'px';
  sel[2].style.width = (r2.x + r2.width) + 'px';
  sel[2].style.height = r2.height + 'px';
  sel[2].style.display = 'block';
  
  elt.scrollIntoView({block:'center'});
}