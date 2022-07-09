function $(a)
{
  return document.getElementById(a);
}

if (!String.prototype.endsWith) 
{
  String.prototype.endsWith = function(searchString, position) 
  {
    var subjectString = this.toString();
    if (typeof position !== 'number' || !isFinite(position) || Math.floor(position) !== position || position > subjectString.length) {
      position = subjectString.length;
    }
    position -= searchString.length;
    var lastIndex = subjectString.indexOf(searchString, position);
    return lastIndex !== -1 && lastIndex === position;
  };
}

Element.prototype.hasClass = function(klass)
{
  if (this.classList)
    return this.classList.contains(klass);
  else
    return !!this.className.match(new RegExp('(\\s|^)' + klass + '(\\s|$)'));
};

Element.prototype.addClass = function(klass)
{
  if (this.classList)
    this.classList.add(klass);
  else if (!this.hasClass(klass))
    this.className += " " + klass;
};

Element.prototype.removeClass = function(klass)
{
  if (this.classList)
    this.classList.remove(klass);
  else if (this.hasClass(klass)) 
  {
    var reg = new RegExp('(\\s|^)' + klass + '(\\s|$)');
    this.className = this.className.replace(reg, ' ');
  }
};

/*Element.prototype.ensureVisible = function()
{
  var parent = this.offsetParent;
  
  while (parent && parent.clientHeight == parent.scrollHeight && parent.clientWidth == parent.scrollWidth)
    parent = parent.offsetParent;
  
  if (parent)
    gw.ensureVisible(this.offsetParent, this.offsetLeft, this.offsetTop, this.offsetWidth, this.offsetHeight);
};*/

gw = {

  version: '0',
  commands: [],
  timers: {},
  windows: [],
  form: '',
  debug: false,
  loaded: {},
  uploads: {},
  autocompletion: [],
  focus: false,
  lock: 0,
  needKeyPress: {},
  
  log: function(msg)
  {
    if (gw.debug)
    {
      if (gw.startTime == undefined)
        gw.startTime = Date.now();
      console.log(((Date.now() - gw.startTime) / 1000).toFixed(3) + ': ' + msg);
    }
  },
  
  getFormId: function(id)
  {
    var pos = id.indexOf(".");
    if (pos > 0)
      return id.substr(0, pos);
  },
  
  load: function(lib)
  {
    var elt, src;
    
    if (gw.loaded[lib])
      return;
    
    if (lib.endsWith('.js'))
    {
      elt = document.createElement('script');
      elt.setAttribute("type", "text/javascript");
      src = $root + '/lib:' + lib.slice(0, -3) + ':' + gw.version + '.js';
      elt.setAttribute("src", src);
    }
    else if (lib.endsWith('.css'))
    {
      elt = document.createElement('link');
      elt.setAttribute("rel", "stylesheet");
      elt.setAttribute("type", "text/css");
      src = $root + '/style:' + lib.slice(0, -4) + ':' + gw.version + '.css';
      elt.setAttribute("href", src);
    }
    else
      return;
      
    document.getElementsByTagName("head")[0].appendChild(elt);
    gw.loaded[lib] = src;
    gw.log('load: ' + src);
  },
  
  setInnerHtml: function(id, html)
  {
    var oldDiv = $(id);
    var newDiv = oldDiv.cloneNode(false);
    newDiv.innerHTML = html;
    oldDiv.parentNode.replaceChild(newDiv, oldDiv);
    gw.onFocus();
  },
  
  setOuterHtml: function(id, html)
  {
    if ($(id))
    {
      $(id).outerHTML = html;
      gw.onFocus();
    }
    else
      gw.log('setOuterHtml: ' + id + '? ' + html);
  },
  
  removeElement: function(id)
  {
    var elt = $(id);
    //for (i = 0; i < id_list.length; i++)
    //{
      //elt = $(id_list[i]);
      if (!elt)
        return;
    
      //console.log(id + " removed");
    
      elt.parentNode.removeChild(elt);
    //}
  },
  
  insertElement: function(id, parent)
  {
    var elt;
    var pelt = $(parent);
    
    if (!pelt)
      return;
      
    elt = document.createElement('div');
    elt.id = id;
    pelt.appendChild(elt);
  },

  setVisible: function(id, visible)
  {
    var elt = $(id);
    if (elt)
    {
      if (visible)
        elt.removeClass('gw-hidden');
      else
        elt.addClass('gw-hidden');
    }
  },
  
  saveFocus: function() {
    var active = document.activeElement.id;
    var selection;
    
    if (active)
      selection = gw.getSelection($(active));
      
    return [active, selection];
  },
  
  restoreFocus: function(save) {
    var elt;
    
    if (save[0])
    {
      elt = $(save[0])
      if (elt) 
      {
        elt.focus();
        gw.setSelection(elt, save[1]);
      }
    }
    //else
    //  gw.active = document.activeElement.id;
  },
  
  wait: function(lock) {
    var elt;
    
    if (lock)
    {
      if (gw.lock == 0)
      {
        elt = $('gw-lock');
        elt.style.zIndex = 1000;
        elt.style.display = 'block';
        
        gw.lock_id = setTimeout(function() {
          $('gw-lock').style.opacity = '1';
          gw.lock_id = undefined;
          }, 500);
      }
      
      gw.lock++;
    }
    else
    {
      gw.lock--;
      if (gw.lock == 0)
      {
        if (gw.lock_id)
        {
          clearTimeout(gw.lock_id);
          gw.lock_id = undefined;
        }
        elt = $('gw-lock');
        elt.style.display = 'none';
        elt.style.opacity = '0';
      }
    }
  },
  
  answer: function(xhr, after)
  {
    if (xhr.readyState == 4)
    {
      if (xhr.status == 200 && xhr.responseText)
      {
        xhr.gw_command && gw.log('==> ' + xhr.gw_command + '...');
        
        gw.focus = false;
        var save = gw.saveFocus();
        
        /*if (gw.debug)
          console.log('--> ' + xhr.responseText);*/
        
        var r = xhr.responseText.split('\n');
        var i, expr;
        
        for (i = 0; i < r.length; i++)
        {
          expr = r[i].trim();
          if (expr.length == 0)
            continue;
          if (gw.debug)
          {
            if (expr.length > 1024)
              gw.log('--> ' + expr.substr(0, 1024) + '...');
            else
              gw.log('--> ' + expr);
          }
          eval(expr);
        }
         
        //eval(xhr.responseText);
        
        if (!gw.focus)
          gw.restoreFocus(save);
        
      }
      
      if (after)
        after();
        
      xhr.gw_command && gw.log('==> ' + xhr.gw_command + ' done.');
      
      if (xhr.gw_command && (xhr.gw_command.length < 5 || xhr.gw_command[4] == undefined || xhr.gw_command[4] == false))
        gw.wait(false);
        
      gw.commands.splice(0, 2);
      gw.sendNewCommand();
    }
  },
  
  sendNewCommand: function()
  {
    var command, after, len;
    var xhr;
    
    for(;;) {
    
      len = gw.commands.length;
      
      if (len < 2)
        return;
      
      command = gw.commands[0];
      after = gw.commands[1];
  
      gw.log('[ ' + command + ' ]');
    
      if (command)
      {
        if (command.length < 5 || command[4] == undefined || command[4] == false)
          gw.wait(true);
          
        xhr = new XMLHttpRequest();
        xhr.gw_command = command;
        xhr.open('GET', $root + '/x?c=' + encodeURIComponent(JSON.stringify(command)), true);
        xhr.onreadystatechange = function() { gw.answer(xhr, after); };
        xhr.send(null);
        gw.log("send XMLHttpRequest...");
        return;
      }
      
      if (after)
        after();
      
      gw.commands.splice(0, 2);
    }
  },

  send: function(command, after)
  {
    gw.log('gw.send: ' + command + ' ' + JSON.stringify(gw.commands));
    
    gw.commands.push(command);
    gw.commands.push(after);
    
    if (gw.commands.length == 2)
      gw.sendNewCommand();
  },

  raise: function(id, event, args, no_wait)
  {
    gw.send(['raise', id, event, args, no_wait]);
  },
  
  update: function(id, prop, value, after)
  {
    gw.send(['update', id, prop, value, true], after);
  },
  
  updateWait: function(id, prop, value, after)
  {
    gw.send(['update', id, prop, value, false], after);
  },
  
  command: function(action)
  {
    gw.send(null, action);
  },

  getSelection: function(o)
  {
    var start, end;
    
    try
    {
      if (o.createTextRange) 
      {
        var r = document.selection.createRange().duplicate();
        r.moveEnd('character', o.value.length)
        if (r.text == '')
          start = o.value.length;
        else
          start = o.value.lastIndexOf(r.text);
        r.moveStart('character', -o.value.length);
        end = r.text.length;
        return [start, end];
      }
      
      if (o.selectionStart && o.selectionEnd)
        return [o.selectionStart, o.selectionEnd];
    }
    catch(e) {};
    
    return undefined;
  },
  
  setSelection: function(o, sel)
  {
    if (sel)
    {
      if (o.setSelectionRange)
        try { o.setSelectionRange(sel[0], sel[1]) } catch(e) {};
    }
  },
  
  onFocus: function()
  {
    var elt = document.activeElement;
    var id, pos, win;
    
    if (gw._focusTimeout)
    {
      clearTimeout(gw._focusTimeout);
      gw._focusTimeout = undefined;
    }
    
    if (elt && elt.id)
    {
      id = elt.id;
      
      if (gw.window.current && !id.startsWith(gw.window.current + '.'))
      {
        gw.setFocus(gw.active);
        return;
      }
      
      gw.active = id;
      gw.update('', '#focus', id);
      //console.log('onFocus: ' + id);
    }
    else
    {
      gw._focusTimeout = setTimeout(function() 
        {
          gw._focusTimeout = undefined;
          //console.log('onFocus: -');
          gw.update('', '#focus', '');
        }, 50);
    }
    
    while (elt && (!elt.id || elt.id.indexOf(':') >= 0))
      elt = elt.parentNode;
    
    if (elt && elt != document.body)
    {
      pos = gw.getPos(elt);
      $('gw-focus').style.top = pos.top + 'px';
      $('gw-focus').style.left = pos.left + 'px';
      $('gw-focus').style.width = pos.width + 'px';
      $('gw-focus').style.height = pos.height + 'px';
      win = gw.getWindow(elt);
      $('gw-focus').style.zIndex = win ? (parseInt(win.style.zIndex) + 2) : 2;
      $('gw-focus').style.display = 'block';
    }
    else
      $('gw-focus').style.display = '';
  },
  
  setFocus: function(id)
  {
    var elt = $(id + ':entry') || $(id);
    
    if (elt)
    {
      elt.focus();
      gw.selection = undefined;
      gw.focus = true;
    }
  },
  
  highlightMandatory: function(id)
  {
    var elt = $(id);
    var elt_br;
    var div;
    var div_br;
    
    if (elt == undefined || elt.gw_mandatory)
      return;
    
    elt.gw_mandatory = div = document.createElement('div');
    div.className = 'gw-mandatory';
    elt.parentNode.insertBefore(div, elt);
    
    elt_br = gw.getPos(elt);
    div_br = gw.getPos(div);
    
    div.style.top = (elt_br.top - div_br.top) + 'px';
    div.style.left = (elt_br.left - div_br.left) + 'px';
    div.style.width = elt_br.width + 'px';
    div.style.height = elt_br.height + 'px';
  },
  
  addTimer: function(id, delay)
  {
    gw.removeTimer(id);
    gw.timers[id] = setInterval(
      function() {
        if (gw.timers[id + '!'])
          return;
        gw.timers[id + '!'] = true;
        gw.send(['raise', id, 'timer', [], true],
          function() {
            if (gw.timers[id])
              gw.timers[id + '!'] = undefined;
            }
          );
      },
      delay);
  },
  
  removeTimer: function(id)
  {
    var t = gw.timers[id];
    if (t)
    {
      clearInterval(gw.timers[id]);
      gw.timers[id] = undefined;
      gw.timers[id + '!'] = undefined;
    }
  },
  
  getTargetId: function(elt)
  {
    for(;;)
    {
      if (elt.id)
        return elt.id;
      elt = elt.parentNode;
      if (!elt)
        return;
    }
  },

  getPos: function(elt)
  {
    var found, left = 0, top = 0, width = 0, height = 0;
    var offsetBase = gw.offsetBase;
   
    if (!offsetBase && document.body) 
    {
      offsetBase = gw.offsetBase = document.createElement('div');
      offsetBase.style.cssText = 'position:absolute;left:0;top:0';
      document.body.appendChild(offsetBase);
    }
    
    if (elt && elt.ownerDocument === document && 'getBoundingClientRect' in elt && offsetBase) 
    {
      var boundingRect = elt.getBoundingClientRect();
      var baseRect = offsetBase.getBoundingClientRect();
      found = true;
      left = boundingRect.left - baseRect.left; //- document.documentElement.scrollLeft;
      top = boundingRect.top - baseRect.top; //- document.documentElement.scrollTop;
      width = boundingRect.right - boundingRect.left;
      height = boundingRect.bottom - boundingRect.top;
    }
    
    return { found: found, left: left, top: top, width: width, height: height, right: left + width, bottom: top + height };
  },
  
  getWindow: function(elt)
  {
    while (!elt.hasClass('gw-window'))
    {
      elt = elt.parentNode;
      if (elt == document.body || !elt)
        return null;
    }
      
    return elt;
  },

  copy: function(elt)
  {
    navigator.clipboard.writeText(elt.value)
      .then(() => {
        // Success!
      })
      .catch(err => {
        console.log('Unable to copy to the clipboard: ', err);
      });
  },
  /*ensureVisible: function(id, x, y, w, h)
  {
    var elt = typeof(id) == 'string' ? $(id) : id;
    var pw, ph,cx, cy, cw, ch;
    var xx, yy, ww, hh;
  
    // WW = W / 2
    ww = w / 2;
    //HH = H / 2
    hh = h / 2;
    // XX = X + WW
    xx = x + ww
    // YY = Y + HH
    yy = y + hh;
    
    // PW = Me.ClientW 
    // PH = Me.ClientH
    pw = elt.clientWidth;
    ph = elt.clientHeight;
    
    cx = - elt.scrollLeft;
    cy = - elt.scrollTop;
    cw = elt.scrollWidth;
    ch = elt.scrollHeight;
  
    //If PW < (WW * 2) Then WW = PW / 2
    //If PH < (HH * 2) Then HH = PH / 2
    if (pw < (ww * 2)) ww = pw / 2;
    if (ph < (hh * 2)) hh = ph / 2;
  
    //If CW <= PW Then
    //  WW = 0
    //  CX = 0
    //Endif
    if (cw <= pw) { ww = 0; cx = 0; }
    
    //If CH <= PH Then
    //  HH = 0
    //  CY = 0
    //Endif
    if (ch <= ph) { hh = 0; cy = 0 }
  
    //If XX < (- CX + WW) Then
    //  CX = Ceil(- XX + WW)
    //Else If XX >= (- CX + PW - WW) Then
    //  CX = Floor(- XX + PW - WW)
    //Endif
    if (xx < (- cx + ww))
      cx = - xx + ww;
    else if (xx >= (- cx + pw - ww))
      cx = - xx + pw - ww;
    
    //If YY < (- CY + HH) Then
    //  CY = Ceil(- YY + HH)
    //Else If YY >= (- CY + PH - HH) Then
    //  CY = Floor(- YY + PH - HH)
    //Endif
    
    if (yy < (- cy + hh))
      cy = - yy + hh;
    else if (yy >= (- cy + ph - hh))
      cy = - yy + ph - hh;
  
    //If CX > 0
    //  CX = 0
    //Else If CX < (PW - CW) And If CW > PW Then
    //  CX = PW - CW
    //Endif
    if (cx > 0)
      cx = 0;
    else if (cx < (pw - cw) && cw > pw)
      cx = pw - cw;
  
    //If CY > 0 Then
    //  CY = 0
    //Else If CY < (PH - CH) And If CH > PH Then
    //  CY = PH - CH
    //Endif
    if (cy > 0)
      cy = 0;
    else if (cy < (ph - ch) && ch > ph)
      cy = ph - ch;
  
    //If $hHBar.Value = - CX And If $hVBar.Value = - CY Then Return True
    //Scroll(- CX, - CY)
    elt.scrollLeft = - cx;
    elt.scrollTop = - cy;
  },*/

  window: 
  {
    zIndex: 0,
    current: '',
    
    exist: function(id)
    {
      return gw.windows.indexOf(id) >= 0;
    },
    
    open: function(id, resizable, modal, minw, minh)
    {
      if (!gw.window.exist(id))
      {
        if (gw.windows.length == 0)
        {
          document.addEventListener('mousemove', gw.window.onMove);
          document.addEventListener('mouseup', gw.window.onUp);
          gw.log('document.addEventListener');
        }
        
        gw.windows.push(id);
        
        $(id).addEventListener('mousedown', gw.window.onMouseDown);
      }
      
      $(id).gw_resizable = resizable;
      $(id).gw_modal = modal;
      $(id).gw_popup = undefined;
      $(id).gw_transient = undefined;
      
      if (modal)
      {
        $(id).gw_current = gw.window.current;
        gw.window.current = id.substr(id.lastIndexOf('@'));;
        
        if ($(id).gw_focus == undefined)
          $(id).gw_focus = gw.saveFocus();
      } 
      
      if (minw != undefined)
      {
        $(id).gw_minw = minw;
        $(id).gw_minh = minh;
      }
      else if ($(id).gw_minw == undefined)
      {
        $(id).gw_minw = $(id).offsetWidth;
        $(id).gw_minh = $(id).offsetHeight;
      }
      
      //console.log('gw.window.open: minw = ' + $(id).gw_minw + ' minh = ' + $(id).gw_minh);
      
      // Touch events 
      //pane.addEventListener('touchstart', onTouchDown);
      //document.addEventListener('touchmove', onTouchMove);
      //document.addEventListener('touchend', onTouchEnd);
      
      gw.window.refresh();
    },
    
    translate: function(id, x, y)
    {
      $(id).style.transform = 'translate(' + (x|0) + 'px,' + (y|0) + 'px)';
      gw.onFocus();
    },
    
    popup: function(id, resizable, control, transient, minw, minh)
    {
      var pos;
      
      if (!gw.window.exist(id))
      {
        if (gw.windows.length == 0)
        {
          document.addEventListener('mousemove', gw.window.onMove);
          document.addEventListener('mouseup', gw.window.onUp);
          gw.log('document.addEventListener');
        }
        
        gw.windows.push(id);
        
        $(id).addEventListener('mousedown', gw.window.onMouseDown);

        pos = gw.getPos($(control));
        //console.log(pos);
        
        /*$(id).style.left = pos.left + 'px';
        $(id).style.top = pos.bottom + 'px';*/
        if (transient)
          gw.window.translate(id, pos.left, pos.top);
        else
          gw.window.translate(id, pos.left, pos.bottom);
      }
      
      $(id).gw_resizable = resizable;
      $(id).gw_modal = true;
      $(id).gw_popup = true;
      $(id).gw_popup_transient = transient;
      
      if ($(id).gw_focus == undefined)
        $(id).gw_focus = gw.saveFocus();
      
      if (minw != undefined)
      {
        $(id).gw_minw = minw;
        $(id).gw_minh = minh;
      }
      else if ($(id).gw_minw == undefined)
      {
        $(id).gw_minw = $(id).offsetWidth;
        $(id).gw_minh = $(id).offsetHeight;
      }
      
      gw.window.refresh();
      
      $(id).scrollIntoView({behavior:"auto", block:"nearest"});
    },

    close: function(id)
    {
      var i;
      
      $(id).removeEventListener('mousedown', gw.window.onMouseDown);
      
      if ($(id).gw_modal)
        gw.window.current = $(id).gw_current;
      
      i = gw.windows.indexOf(id);
      if (i >= 0)
      {
        gw.windows.splice(i, 1);
        gw.window.refresh();
      }
      
      if ($(id).gw_focus)
      {
        gw.restoreFocus($(id).gw_focus);
        $(id).gw_focus = undefined;
      }
      
      $(id).gw_minw = $(id).gw_minh = undefined;
    },
    
    refresh: function()
    {
      var i = 0;
      var zi;
      
      while (i < gw.windows.length)
      {
        if ($(gw.windows[i]))
        {
          zi = 11 + i * 4;
          if ($(gw.windows[i]).style.zIndex != zi)
            $(gw.windows[i]).style.zIndex = zi;
          i++;
        }
        else
          gw.windows.splice(i, 1);
      }
      
      gw.window.updateModal();
      
      if (gw.windows.length == 0)
      {
        gw.log('document.removeEventListener');
        document.removeEventListener('mousemove', gw.window.onMove);
        document.removeEventListener('mouseup', gw.window.onUp);
      }
      else
        gw.window.updateTitleBars();
        
      gw.onFocus();
    },
    
    updateTitleBars: function()
    {
      var i, win, last;
      
      for (i = 0; i < gw.windows.length; i++)
      {
        win = gw.windows[i];
        if ($(win).gw_popup)
          continue;
        $(win).addClass('gw-deactivated');
        $(win + '-titlebar').addClass('gw-deactivated');
        last = win;
      }
      
      if (last && !$(last).gw_popup)
      {
        $(last).removeClass('gw-deactivated');
        $(last + '-titlebar').removeClass('gw-deactivated');
      }
    },
    
    raise: function(id, send)
    {
      var i = gw.windows.indexOf(id);
      if (i < 0)
        return;
      
      gw.windows.splice(i, 1);
      gw.windows.push(id);
      
      for (i = 0; i < gw.windows.length; i++)
        $(gw.windows[i]).style.zIndex = 11 + i * 4;
        
      gw.window.updateTitleBars();
      $(id).focus();
        
      if (send)
        gw.update('', '#windows', gw.windows);
    },
    
    updateModal: function()
    {
      var i, elt = $('gw-modal');
      
      for (i = gw.windows.length - 1; i >= 0; i--)
      {
        if ($(gw.windows[i]).gw_modal)
        {
          gw.window.zIndex = 10 + i * 4;
          elt.style.zIndex = 10 + i * 4;
          elt.style.display = 'block';
          /*if ($(gw.windows[i]).gw_popup)
            elt.style.opacity = '0';
          else
            elt.style.opacity = '';*/
          gw.onFocus();
          return;
        }
      }
      
      gw.window.zIndex = 0;
      elt.style.display = 'none';
      gw.onFocus();
    },
    
    center: function(id)
    {
      gw.window.translate(id, (window.innerWidth - $(id).offsetWidth) / 2, (window.innerHeight - $(id).offsetHeight) / 2);
      gw.window.updateGeometry(id);
    },
    
    isMaximized: function(id)
    {
      return $(id).gw_save_geometry != undefined;
    },
    
    maximize: function(id)
    {
      var geom = $(id).gw_save_geometry;
      if (geom != undefined)
      {
        //$(id).style.left = geom[0];
        //$(id).style.top = geom[1];
        $(id).style.transform = geom[0]
        $(id).style.width = geom[1];
        $(id).style.height = geom[2];
        $(id).gw_save_geometry = undefined;
      }
      else
      {
        $(id).gw_save_geometry = [$(id).style.transform, $(id).style.width, $(id).style.height];
        $(id).style.transform = '';
        $(id).style.width = '100%';
        $(id).style.height = '100%';
      }
      gw.window.updateGeometry(id);
    },
    
    onMouseDown: function(e)
    {
      gw.window.onDown(e);
    },
    
    onDown: function(e)
    {
      var c, win;
      
      gw.window.context = undefined;
      
      if (e.target.className == 'gw-window-button')
        return;
        
      gw.window.onMove(e);
      
      c = gw.window.context;
      if (c == undefined)
        return; 
        
      if ($(c.id).gw_save_geometry)
        return;
        
      if (c.isMoving || c.isResizing)
      {
        gw.window.raise(c.id);
        gw.window.downEvent = e;
        e.preventDefault();
      }
    },
    
    onDownModal: function(transient)
    {
      var win = gw.windows[gw.windows.length - 1];
      
      if ($(win).gw_popup)
      {
        if (transient && !$(win).gw_popup_transient)
          return;
        gw.update(win, '#close');
      }
    },
    
    onMove: function(e) 
    {
      var i, id, elt, b, x, y, bx, by, bw, bh, th;
      var onTopEdge, onLeftEdge, onRightEdge, onBottomEdge, isResizing;
      var MARGINS = 6;
      
      if (gw.window.downEvent)
      {
        gw.window.context.cx = e.clientX;
        gw.window.context.cy = e.clientY;
        gw.window.animate();
        return;
      }
      
      gw.window.context = undefined;
      
      for (i = 0; i < gw.windows.length; i++)
      {
        id = gw.windows[gw.windows.length - i - 1];
        elt = $(id);
        
        if (elt.style.zIndex < gw.window.zIndex)
          continue;
        
        b = elt.getBoundingClientRect();
        
        bx = b.left; // - MARGINS;
        by = b.top; // - MARGINS;
        bw = b.width; // + MARGINS * 2;
        bh = b.height; // + MARGINS * 2;
        
        x = e.clientX - bx;
        y = e.clientY - by;
        
        //console.log(x + ',' + y + ' : ' + bx + ',' + by + ',' + bw + ',' + bh);
        
        if (x >= 0 && x < bw && y >= 0 && y < bh)
        {
          if (elt.gw_resizable)
          {
            onTopEdge = y < MARGINS;
            onLeftEdge = x < MARGINS;
            onRightEdge = x >= (bw - MARGINS);
            onBottomEdge = y >= (bh - MARGINS);
            
            isResizing = onTopEdge || onLeftEdge || onRightEdge || onBottomEdge;
          }
          else
            onTopEdge = onLeftEdge = onRightEdge = onBottomEdge = isResizing = false;
          
          if ($(id).gw_popup)
            th = 0;
          else
            th = $(id + '-titlebar').offsetHeight;
          isMoving = !isResizing && y < (th + MARGINS);
          
          gw.window.context = {
            id: id,
            x: b.left + window.scrollX,
            y: b.top + window.scrollY,
            cx: e.clientX,
            cy: e.clientY,
            w: b.width,
            h: b.height,
            isResizing: isResizing,
            isMoving: isMoving,
            onTopEdge: onTopEdge,
            onLeftEdge: onLeftEdge,
            onRightEdge: onRightEdge,
            onBottomEdge: onBottomEdge
            };
          gw.window.animate();
          break;
        }
      }
    },
    
    updateGeometry: function(id)
    {
      var b = $(id).getBoundingClientRect();
      gw.update(id, '#geometry', [ b.left + 'px', b.top + 'px', b.width + 'px', b.height + 'px', gw.window.isMaximized(id)]);
      gw.onFocus();
    },
    
    onUp: function(e)
    {
      var c = gw.window.context;
      
      gw.window.downEvent = undefined;
      
      if (c && (c.isMoving || c.isResizing))
      {
        var id = gw.window.context.id;
        gw.window.context = undefined;
        gw.window.raise(id, true);
        gw.window.updateGeometry(id);
      }
    },

    animate: function() 
    {
      var id, elt, c, e, x, y, w, h;
      var minWidth;
      var minHeight;
      
      //requestAnimationFrame(gw.window.animate);
      
      c = gw.window.context;
      if (!c) return;
    
      elt = $(c.id);
      minWidth = elt.gw_minw;
      minHeight = elt.gw_minh; //$(c.id + '-titlebar').offsetHeight + 2 + elt.gw_minh;
      e = gw.window.downEvent;
    
      if (c && c.isResizing && e)
      {
        if (c.onRightEdge)
          elt.style.width = Math.max(c.w + c.cx - e.clientX, minWidth) + 'px';
        
        if (c.onBottomEdge)
          elt.style.height = Math.max(c.h + c.cy - e.clientY, minHeight) + 'px';
    
        x = c.x;
        y = c.y;
    
        if (c.onLeftEdge) 
        {
          x = c.x + c.cx - e.clientX;
          w = c.x + c.w - x;
          if (w >= minWidth) 
          {
            elt.style.width = w + 'px';
            //elt.style.left = x + 'px'; 
            gw.window.translate(c.id, x, y);
            //c.x = x;
          }
        }
    
        if (c.onTopEdge) 
        {
          y = c.y + c.cy - e.clientY;
          h = c.y + c.h - y;
          if (h >= minHeight) 
          {
            elt.style.height = h + 'px';
            //elt.style.top = y + 'px'; 
            gw.window.translate(c.id, x, y);
          }
        }
    
        return;
      }
    
      if (c && c.isMoving && e)
      {
        /*elt.style.left = (Math.max(0, c.x + c.cx - e.clientX)) + 'px';
        elt.style.top = (Math.max(0, c.y + c.cy - e.clientY)) + 'px';*/
        gw.window.translate(c.id, Math.max(0, c.x + c.cx - e.clientX), Math.min(window.innerHeight - elt.firstElementChild.offsetHeight, Math.max(0, c.y + c.cy - e.clientY)));
        return;
      }
    
      // This code executes when mouse moves without clicking
    
      if (c.onRightEdge && c.onBottomEdge || c.onLeftEdge && c.onTopEdge)
        elt.style.cursor = 'nwse-resize';
      else if (c.onRightEdge && c.onTopEdge || c.onBottomEdge && c.onLeftEdge)
        elt.style.cursor = 'nesw-resize';
      else if (c.onRightEdge || c.onLeftEdge)
        elt.style.cursor = 'ew-resize';
      else if (c.onBottomEdge || c.onTopEdge)
        elt.style.cursor = 'ns-resize';
      else
        elt.style.cursor = '';
    }
  },
  
  menu:
  {
    hide: function(elt)
    {
      elt.style.display = 'none';
      setTimeout(function() { elt.style.display = ''; }, 150);
    },
    
    click: function(name, event)
    {
      var id = gw.getTargetId(event.target);
      gw.update(name, '#click', id);
      event.stopPropagation();
    }
  },
  
  table:
  {
    selectRange: function(id, start, end, checked)
    {
      var i;
      var tr;
      
      if ($(id).hasClass('gw-table'))
      {
        if (end < start)
        {
          i = start;
          start = end;
          end = i;
        }
        
        for (i = start; i <= end; i++)
        {
          tr = $(id + ':' + i);
          if (checked)
            tr.addClass('gw-selected');
          else
            tr.removeClass('gw-selected');
        }
          
        gw.update(id, '!' + start + ':' + end, checked);
      }
      else
      {
        tr = $(id + ':' + start);
        if (checked)
          tr.addClass('gw-selected');
        else
          tr.removeClass('gw-selected');
        
        gw.update(id, '!' + start, checked);
      }
    },
  
    select: function(id, row, event)
    {
      var elt = $(id + ':' + row);
      var last = $(id).gw_current;
      var selected = !elt.hasClass('gw-selected');
      
      if (event)
      {
        if (event.shiftKey && last)
          gw.table.selectRange(id, last, row, selected);
        else
          gw.table.selectRange(id, row, row, selected);
      }
      else
      {
        if (last != undefined)
          $(id + ':' + last) && $(id + ':' + last).removeClass('gw-selected');
        elt.addClass('gw-selected');
        gw.update(id, '$' + row, null);
      }
      
      $(id).gw_current = row;
      
      $(id).addClass('gw-unselectable');
      setTimeout(function() { $(id).removeClass('gw-unselectable'); }, 0);
    },
    
    checkRange: function(id, start, end, checked)
    {
      if ($(id).hasClass('gw-table'))
      {
        if (end < start)
        {
          var i = start;
          start = end;
          end = i;
        }
        
        for (i = start; i <= end; i++)
          $(id + ':c:' + i).checked = checked;
      
        gw.update(id, '!' + start + ':' + end, checked);
      }
      else
      {
        $(id + ':c:' + start).checked = checked;
        gw.update(id, '!' + start, checked);
      }
    },
  
    check: function(id, row, event)
    {
      var elt = $(id + ':c:' + row);
      var checked = !elt.checked;
      var last = $(id).gw_current;
      var len;
      
      elt.focus();
      
      if (event && event.shiftKey && last)
        gw.table.checkRange(id, last, row, checked);
      else
        gw.table.checkRange(id, row, row, checked);
      
      if (event.target == elt)
        elt.checked = !checked;

      $(id).gw_current = row;

      $(id).addClass('gw-unselectable');
      setTimeout(function() { $(id).removeClass('gw-unselectable'); }, 0);
    },
    
    ensureVisible: function(id, row)
    {
      var sw = $(id).firstChild;
      gw.scrollview.scroll(id, sw.scrollLeft, $(id + ':' + row).offsetTop - sw.clientHeight / 2);
    }
  },
  
  tree:
  {
    expand: function(id, key, open, event)
    {
      gw.update(id, '^' + key, open);
      if (event) event.stopPropagation();
    },
  },
  
  scrollview:
  {
    setHeaders: function(id, hid, vid)
    {
      $(id).gw_headerh = hid;
      $(id).gw_headerv = vid;
    },

    ensureVisible: function(id, child)
    {
      var sw = $(id).firstChild;
      child = $(child);
      gw.scrollview.scroll(id, child.offsetLeft - (sw.clientWidth - child.offsetWidth) / 2, child.offsetTop - (sw.clientHeight - child.offsetHeight) / 2);
    },
    
    getView: function(elt)
    {
      if (elt.hasClass('gw-listbox'))
        return elt;
      else
        return elt.firstChild;
    },
    
    onScroll: function(id, more, timeout)
    {
      var sw, last;
      var elt = $(id);
      
      if (!elt)
        return;
      
      sw = gw.scrollview.getView(elt);
      last = elt.gw_last_scroll;
      
      if (last && last[0] == sw.scrollLeft && last[1] == sw.scrollTop)
        return;
      
      elt.gw_last_scroll = [sw.scrollLeft, sw.scrollTop];
      
      gw.log('gw.scrollview.onScroll: ' + id + ' ' + sw.scrollLeft + ',' + sw.scrollTop);
      
      if (more)
      {
        //if ((sw.scrollHeight - sw.scrollTop) === (sw.clientHeight))
        if (sw.scrollTop >= (sw.scrollHeight - sw.clientHeight - 16))
        {
          /*var wait = document.createElement('div');
          wait.className = 'gw-waiting';
          elt.appendChild(wait);*/
          if (elt.gw_scroll)
          {
            clearTimeout(elt.gw_scroll); 
            elt.gw_scroll = undefined;
          }
          
          gw.update(id, '#more', [sw.scrollLeft, sw.scrollTop]);
          return;
        }
      }
      
      if (elt.gw_headerh)
        $(elt.gw_headerh).firstChild.scrollLeft = sw.scrollLeft;

      if (elt.gw_headerv)
        $(elt.gw_headerv).firstChild.scrollTop = sw.scrollTop;

      if (elt.gw_noscroll)
      {
        elt.gw_noscroll = undefined;
        return;
      }
      
      if (elt.gw_scroll)
        clearTimeout(elt.gw_scroll); 
      
      elt.gw_scroll = setTimeout(function()
        { 
          var pos = [sw.scrollLeft, sw.scrollTop];
          
          gw.log('gw.control.onScroll (timer): ' + id + ' ' + sw.scrollLeft + ',' + sw.scrollTop);
          clearTimeout(elt.gw_scroll); 
          
          gw.update(elt.id, '#scroll', pos, function() 
            { 
              elt.gw_scroll = undefined; 
              if (pos[0] != sw.scrollLeft || pos[1] != sw.scrollTop)
                gw.scrollview.onScroll(id, more, timeout);
            });
            
          //elt.gw_scroll = undefined;
        }, 250);
    },
    
    scroll: function(id, x, y)
    {
      var sw = gw.scrollview.getView($(id));
      
      gw.log("gw.control.scroll: " + id + ": " + x + " " + y);
      
      if (x != sw.scrollLeft)
      {
        $(id).gw_noscroll = true;
        sw.scrollLeft = x;
      }
      if (y != sw.scrollTop)
      {
        $(id).gw_noscroll = true;
        sw.scrollTop = y;
      }
      if (x != sw.scrollLeft || y != sw.scrollTop)
        gw.update(id, '#scroll', [sw.scrollLeft, sw.scrollTop]); 
    }
  },
  
  file: 
  {
    select: function(id) 
    {
      var elt = $(id + ':file');
      
      if ($(id).gw_uploading)
        return;
      
      elt.focus();
      elt.click();
    },
    
    finish: function(xhr)
    {
      if (xhr.gw_progress)
      {
        setTimeout(function() { gw.file.finish(xhr); }, 250);
        return;
      }
      
      gw.update(xhr.gw_id, 'progress', 1, function() {
        gw.answer(xhr); 
        gw.uploads[xhr.gw_id] = undefined;
        //gw.update(xhr.gw_id, 'finish', []);
        xhr.gw_id = undefined;
        });
    },
    
    upload: function(id, key)
    {
      var elt = $(id + ':file');
      var file = elt.files[0];
      var xhr = new XMLHttpRequest();
      var form = new FormData();
      
      if (gw.uploads[id])
        return;
      
      gw.uploads[id] = xhr;
      
      //gw.log('gw.file.upload: ' + id + ': ' + file.name);
      
      //xhr.gw_progress = 0;
      
      xhr.gw_progress = 1;
      gw.update(id, 'progress', 0, function() { xhr.gw_progress--; });
      
      form.append('file', file);
      form.append('name', file.name);
      form.append('id', id);
      
      //xhr.upload.addEventListener("loadstart", loadStartFunction, false);  
      //xhr.upload.addEventListener("load", transferCompleteFunction, false);
      
      xhr.upload.addEventListener('progress', 
        function(e) 
        {
          //console.log('upload: progress ' + e.loaded + ' / ' + e.total + ' ' + xhr.gw_id + ' ' + e.lengthComputable + ' ' + xhr.gw_progress);
          
          if (xhr.gw_id == undefined)
            return;
            
          if (e.lengthComputable)
          {
            var t = (new Date()).getTime();
            
            if ((xhr.gw_time == undefined || (t - xhr.gw_time) > 50) && xhr.gw_progress == 0)
            {
              xhr.gw_progress++;
              gw.update(xhr.gw_id, 'progress', e.loaded / e.total, function() { xhr.gw_progress--; }); 
              xhr.gw_time = t;
            }
          }
        },
        false);
      
      xhr.gw_command = ['upload', id];
      xhr.gw_id = id;
        
      xhr.open('POST', $root + '/upload:' + key, true);  
      
      xhr.onreadystatechange = function() 
        {
          if (xhr.readyState == 4)
            gw.file.finish(xhr);
        };
        
      xhr.send(form);  
    },
    
    abort: function(id)
    {
      if (gw.uploads[id])
        gw.uploads[id].abort();
    }
  },

  autocomplete: function(id)
  {
    new AutoComplete({
      selector: $(id + ':entry'),
      cache: false,
      source: function(term, response) {
        var xhr = $(id).gw_xhr;
        if (xhr)
        {
          try { xhr.abort(); } catch(e) {}
        }
        
        $(id).gw_xhr = xhr = new XMLHttpRequest();
        
        xhr.open('GET', $root + '/x?c=' + encodeURIComponent(JSON.stringify(['raise', id, 'completion', [term]])), true);
        xhr.onreadystatechange = function() {
          if (xhr.readyState == 4)
          {
            gw.autocompletion = [];
            gw.answer(xhr);
            response(gw.autocompletion);
          }
        };
        xhr.send();
      },
      onSelect: function(e, term, item) {
        gw.textbox.setText(id, gw.textbox.getText(id));
      }
    });
  },
  
  textbox:
  {
    onActivate: function(id, e)
    {
      if (e.keyCode == 13)
        setTimeout(function() { gw.update(id, 'text', $(id + ':entry').value); gw.raise(id, 'activate', [], false); }, 20);
    },
    
    onChange: function(id)
    {
      if ($(id).gw_timer) clearTimeout($(id).gw_timer);
      $(id).gw_timer = setTimeout(function() { gw.update(id, 'change', $(id + ':entry').value, null); }, 20);
    },
    
    getText: function(id)
    {
      return $(id + ':entry').value;
    },
    
    moveEnd: function(id)
    {
      gw.setSelection($(id + ':entry'), [-1, -1]);
    },
    
    setText: function(id, text)
    {
      gw.command(function() {
        $(id + ':entry').value = text;
        gw.textbox.moveEnd(id);
        gw.update(id, 'text', text);
        });
    },
    
    clear: function(id)
    {
      gw.textbox.setText(id, '');
      gw.setFocus(id);
      gw.raise(id, 'activate', [], false);
    },
    
    copy: function(id)
    {
      gw.copy($(id + ':entry'));
    }
  },

  textarea:
  {
    onChange: function(id)
    {
      if ($(id).gw_timer) clearTimeout($(id).gw_timer);
      $(id).gw_timer = setTimeout(function() { gw.update(id, 'change', $(id).value, null); }, 50);
    },
    
    copy: function(id)
    {
      gw.copy($(id));
    },
    
    moveEnd: function(id)
    {
      gw.setSelection($(id), [-1, -1]);
      $(id).scrollTop = $(id).scrollHeight;
    },
    
    setText: function(id, text)
    {
      gw.command(function() {
        $(id).value = text;
        gw.textarea.moveEnd(id);
        gw.update(id, 'text', text);
        });
    },
  },
  
  button:
  {
    setText: function(id, text)
    {
      $(id).lastElementChild.innerHTML = text;
    },
    
    click: function(id)
    {
      $(id).focus();
      $(id).click();
    }
  },
  
  combobox:
  {
    resize: function(id)
    {
      $(id + ':select').onmouseover = function() { $(id + ':select').style.width = $(id).offsetWidth + 'px'; }
    },
    
    update: function(id, index, text)
    {
      if (text != undefined)
        $(id + ':entry').value = text;
        
      $(id + ':select').selectedIndex = index;
    }
  },
  
  listbox:
  {
    selectRange: function(id, start, end, checked)
    {
      var items = $(id).children;
      var i;
      var elt;
      
      if (end < start)
      {
        i = start;
        start = end;
        end = i;
      }
      
      for (i = start; i <= end; i++)
      {
        elt = items[i];
        if (checked)
          elt.addClass('gw-selected');
        else
          elt.removeClass('gw-selected');
      }
        
      gw.update(id, checked ? '+' : '-',  [start, end - start + 1]);
    },
  
    select: function(id, row, event, multiple)
    {
      var items = $(id).children;
      var last = $(id).gw_current;
      var elt = items[row];
      var selected;
      
      if (event.detail == 2 || !elt)
        return;
      
      selected = !elt.hasClass('gw-selected');
      
      if (multiple)
      {
        if (event.shiftKey && last)
          gw.listbox.selectRange(id, last, row, selected);
        else
          gw.listbox.selectRange(id, row, row, selected);
      }
      else
      {
        if (last != undefined)
          items[last] && items[last].removeClass('gw-selected');
        elt.addClass('gw-selected');
        gw.listbox.ensureVisible(id, row);
        gw.update(id, '$', row);
      }
      
      $(id).gw_current = row;
      
      /*$(id).addClass('gw-unselectable');
      setTimeout(function() { $(id).removeClass('gw-unselectable'); }, 0);*/
    },
    
    onKeyDown: function(id, event)
    {
      var row;
      
      if (event.key == 'ArrowDown')
      {
        if (!(event.altKey || event.shiftKey || event.ctrlKey))
        {
          row = $(id).gw_current;
          gw.listbox.select(id, row + 1, event, false);
          event.preventDefault();
        }
      }
      else if (event.key == 'ArrowUp')
      {
        if (!(event.altKey || event.shiftKey || event.ctrlKey))
        {
          row = $(id).gw_current;
          gw.listbox.select(id, row - 1, event, false);
          event.preventDefault();
        }
      }
    },
    
    ensureVisible: function(id, row)
    {
      var elt = $(id);
      var child = elt.children[row];
      
      //console.log(row + ' / ' + (child.offsetTop + child.offsetHeight) + ' / ' + elt.scrollTop + ' / ' + elt.clientHeight);
      
      if ((child.offsetTop + child.offsetHeight) >= (elt.scrollTop + elt.clientHeight))
        gw.scrollview.scroll(id, elt.scrollLeft, child.offsetTop + child.offsetHeight - elt.clientHeight);
      else if (child.offsetTop < elt.scrollTop)
        gw.scrollview.scroll(id, elt.scrollLeft, child.offsetTop);
    }
  },
  
  image:
  {
    preload: function(images)
    {
      var image;
      var i;
      
      for (i = 0; i < images.length; i++)
      {
        image = new Image();
        image.src = images[i];
      }
    }
  },
  
  sound:
  {
    pause: function(id)
    {
      var elt = $(id);
      elt.pause();
    },
    
    stop: function(id)
    {
      var elt = $(id);
      elt.pause();
      elt.currentTime = 0;
    },
    
    play: function(id)
    {
      var elt = $(id);
      elt.pause();
      elt.currentTime = 0;
      elt.play();
    }
  },
  
  makeShortcut: function(event)
  {
    var shortcut = '';
    
    if (event.ctrlKey && event.key != 'Control') shortcut += 'CTRL+';
    if (event.shiftKey && event.key != 'Shift') shortcut += 'SHIFT+';
    if (event.altKey && event.key != 'Alt') shortcut += 'ALT+';
    if (event.meta && event.key != 'Meta') shortcut += 'META+';
    shortcut += event.key.toUpperCase();
    return shortcut;
  },
  
  sendKeyPress: function(event, id)
  {
    gw.send(['keypress', id, 
      {
        'altKey': event.altKey,
        'ctrlKey': event.ctrlKey,
        'key': event.key,
        'metaKey': event.meta,
        'shiftKey': event.shiftKey
      }],
      null);
  },
  
  onKeyDown: function(event)
  {
    var elt;
    var id;
    
    if (event.bubbles && gw.shortcuts)
    {
      var shortcut = gw.makeShortcut(event);
      if (gw.shortcuts[shortcut])
      {
        gw.log('shortcut -> ' + shortcut);
        gw.sendKeyPress(event, '');
        event.preventDefault();
        return;
      }
    }
    
    if (!event.bubbles)
      return;
    
    id = '';
    elt = document.activeElement;
    while (elt) // TODO: stop at window?
    {
      id = elt.id;
      if (id && id.indexOf(':') < 0)
        break;
      elt = elt.parentNode;
    }
  
    if (id && (gw.needKeyPress[id] != undefined || gw.needKeyPress[gw.getFormId(id)]))
      gw.sendKeyPress(event, id);
  },
}

document.onkeydown = gw.onKeyDown;
document.addEventListener('focusin', gw.onFocus);
document.addEventListener('focusout', gw.onFocus);

