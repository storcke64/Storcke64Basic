[WELCOME]

<h3>Добро пожаловать в <b>Gambas</b>!</h3>

<p><b>Gambas</b> - это графическая среда разработки,
основанная на продвинутом интерпретаторе <i>Basic</i>.</p>

<p>Цель <b>Gambas</b> - дать вам возможность легко и
быстро создавать мощные программы. Но чистые
программы остаются на <i>вашей</i> ответственности...</p>

<p>Наслаждайтесь!</p>

<p align="right">Beno&icirc;t Minisini<br><u>g4mba5@gmail.com</u><br><br><br></p>


[STARTUP]

<h3>Класс запуска</h3>

<p>Каждый проект должен иметь <i>класс запуска</i>. Этот класс
запуска должен определять статический открытый метод
с именем <tt><b>Main</b></tt> без аргументов, который будет действовать
как метод запуска вашей программы.</p>

<p>Вы можете определить класс запуска, щёлкнув по
нему правой кнопкой мыши в дереве проекта и
выбрав <u>класс запуска</u> во всплывающем меню.</p>

<p>Нет необходимости определять метод <tt>Main</tt> в форме
запуска, поскольку он уже имеет предопределённый метод.</p>

<p>Этот предопределённый метод запуска создаёт экземпляр формы и показывает её как в
<i>Visual Basic&trade;</i>.</p>


[OPEN]

<h3>Открыть</h3>

<p>Инструкция <b>Open</b> в <b>Gambas</b> не работает
как в <i>Visual Basic&trade;</i>. Она не возвращает файл как целое число,
а как объект <b>File</b>.</p>

<p>Итак, вместо того, чтобы написать:</p>

<pre>Dim handle As Integer
...
Open "myfile" For Read As #handle</pre>

<p>Вы должны написать:</p>

<pre>Dim handle As File
...
handle = Open "myfile" For Read</pre>


[CATDIR]

<h3>Конкатенация путей</h3>

<p>Знаете ли вы, что вы можете объединить имена директорий
и файлов с помощью оператора <b><tt>&amp;/</tt></b>? Этот
оператор имеет дело с косыми чертами, так что получающийся
путь идеален.</p>

<p>Например:</p>

<pre>Print "/home/gambas" &amp;/ ".bashrc"
&rarr; /home/gambas/.bashrc

Print "/home/gambas/" &amp;/ "/tmp" &amp;/ "foo.bar"
&rarr; /home/gambas/tmp/foo.bar</pre>

<p>Разве это не чудесно?</p>


[EXEC]

<h3>Исполняемый файл</h3>

<p>Вы можете создать исполняемый файл из всего вашего проекта.
Выберите <u>Создать исполняемый файл</u> в меню <u>Проект</u>.</p>

<p>Когда <b>Gambas</b> создаёт исполняемый файл, он по
умолчанию помещает результат в директорию вашего проекта.
Имя исполняемого файла совпадает с именем вашего проекта.</p>


[PATH]

<h3>Относительные пути</h3>

<p>Относительные пути имеют особое значение в <b>Gambas</b>.
Они всегда ссылаются на файлы внутри ваших проектов.</p>
<p>Нет понятия <i>текущей директории</i> и нет ключевого слова, такого как <tt>CHDIR</tt>, чтобы изменить её.</p>
<p><b>Будьте осторожны:</b> вы должны использовать относительные пути
только для доступа к файлам проекта, потому что абсолютные пути
больше не будут работать, когда вы создаёте исполняемый файл.</p>


[GLOBAL]

<h3>Глобальные переменные</h3>

В <b>Gambas</b> <b>нет глобальных переменных</b>!
<p>В качестве обходного пути, поместите их в свой основной модуль
и объявите их как <tt>Public</tt>.</p>
<p>Если у вас нет основного модуля main в вашем проекте, но
есть основная форма, то объявите их как <tt>Static Public</tt>.</p>
<p>Чтобы получить доступ к этим переменным, необходимо использовать
имя основного модуля main или форму: <tt>MyMainModule.MyGlobalVariable</tt>
или <tt>MyMainForm.MyGlobalVariable</tt>.</p>


[EMPTY]

<h3>Пустые строки</h3>

<p>Чтобы узнать, является ли строка пустой, нет необходимости использовать
функцию <tt><b>Len()</b></tt>. Вы можете непосредственно проверить это,
так как пустая строка - <tt><b>False</b></tt>, а непустая строка - <tt><b>True</b></tt>.</p>

<p>Например, вместо того, чтобы делать:</p>

<pre>If Len(MyString) &gt; 0 Then ...
If Len(MyString) = 0 Then ...</pre>

<p>Вы должны делать:</p>

<pre>If MyString THEN ...
If Not MyString THEN ...</pre>


[TRANSLATE]

<h3>Перевод</h3>

<p>Приложения Gambas полностью переводимое, при условии, что вы
сообщаете ему, какие строки должны быть переведены, а какие нет.</p>
<p>Чтобы пометить строки как переводимые, просто заключите их в круглые скобки:</p>

<pre>Print ("Translate me")
Print "But do not translate me!"</pre>


[SUBST]

<h3>Subst$</h3>

<p>Функция <b><tt>Subst$()</tt></b> очень полезна для интернационализации вашего приложения.</p>

<p>Требуется как минимум два аргумента. Первый - это текстовая маска, к которой применяется подстановка.
Другие являются аргументами замещения, пронумерованными от одного.</p>

<p>Каждый <tt>&amp;X</tt> шаблон в строке замены будет заменён X-<sup>ым</sup> аргументом замены.
Например:</p>

<pre>Print Subst(("Substitution of &amp;1, &amp;2 and &amp;3"),
  "first", "second", "third")

&rarr; Substitution of first, second and third</pre>


[EVENT]

<h3>Обработчики событий</h3>

<p>Каждый элемент управления и каждый объект, который может вызывать
события, имеет <i>наблюдателя событий</i> и <i>имя группы</i> событий.</p>

<p>Наблюдатель событий перехватывает каждое событие, вызванное объектом,
а имя группы событий является префиксом процедуры, вызываемой для
управления событием. Эта функция называется <i>обработчиком событий</i>.</p>

<p>По умолчанию этот наблюдатель событий является объектом, в котором вы
создали элемент управления, а имя группы - это имя элемента управления.</p>

<p>Таким образом, форма получает все события, созданные
элементами управления, которые вы создали внутри.</p>

<pre>' Gambas form
Dim hButton As Button

Public Sub _new()
&nbsp;&nbsp;hButton = New Button(Me) As "MyButton"
End

Public Sub MyButton_Click()
&nbsp;&nbsp;Print "You have clicked MyButton !"
End</pre>


[GROUP]

<h3>Группы элементов управления</h3>

<p>Каждый элемент управления имеет свойство <b>(Group)</b>. Когда это
свойство установлено, префикс имени обработчика события - это
имя группы, а не имя элемента управления.</p>

<p>Предположим, у вас есть <b>кнопка</b> с именем <tt>btnAction</tt> со следующим обработчиком события <b>Click</b>:</p>

<pre>Public SubbtnAction_Click()</pre>

<p>Если вы установите свойство <b>(Group)</b> для <tt>btnAction</tt>
в значение <tt>"MyGroup"</tt>, то обработчик событий, который будет
получать события от кнопки, будет выглядеть следующим образом:</p>

<pre>Pubic Sub MyGroup_Click()</pre>

<p>Это свойство позволяет обрабатывать события различных элементов
управления в одной функции. И элементы управления одной и той же
группы не обязательно должны быть одного типа!</p>

<p><b>Примечание:</b> Старый ветеран <b>Visual Basic&trade;</b> может
распознавать концепцию <i>управляющего массива</i>, но в более мощной
реализации. :-)</p>


[FORM]

<h3>Формы</h3>

<p>В <b>Gambas</b> форма является собственным наблюдателем событий,
так что вы можете напрямую управлять её событиями (такими как
<b>Resize</b>, <b>Activate</b>, ...) в своём собственном коде класса.</p>

<p>Таким образом, новички из <b>Visual Basic&trade;</b>
не дезориентированы :-).</p>


[EMBED]

<h3>Встраивание форм</h3>

<p>Вы можете встроить любую форму в другие формы с <b>Gambas</b>!</p>

<p>Чтобы сделать такую мощную вещь, просто создайте экземпляр формы,
передав родительский контейнер в качестве последнего аргумента конструктора.</p>

<p>Например:</p>
<pre>Dim hForm As MyDialog
Dim hSuperControl As MyForm
' Создать диалог
hForm = New MyDialog
' Вставьте форму в этот диалог
' Обратите внимание, что эта форма принимает два параметра перед контейнером
hSuperControl = New MyForm(Param1, Param2, MyDialog)
' Переместить и изменить размер формы
hSuperControl.Move(8, 8, 128, 64)</pre>

<p><b>Будьте осторожны:</b> форма, встроенная в другую форму, всё ещё является формой, как и её собственный наблюдатель событий.</p>


[TAG]

<h3>Свойство тега</h3>

<p>Каждый элемент управления имеет свойство <b>Tag</b>. Это свойство
предназначено для программиста и может содержать любые данные
<b>Variant</b>, которые вы считаете уместными.</p>

<p>Это очень полезно, когда вы хотите различить элементы управления
одной и той же группы в общем обработчике событий.</p>


[LAST]

<h3>Последний</h3>

<p>Ключевое слово <b>Last</b> возвращает последний элемент управления,
который получил событие. Это очень полезно, когда вы хотите написать
обработчик событий, который не зависит от имени элемента управления.</p>

<p>Например, предположим, что вы хотите написать калькулятор.
Вы определили десять кнопок, по одной для каждой цифры, каждая
в одной <i>группе элементов управления</i> "Digit". <b>Tag</b> каждого элемента
управления устанавливается на цифру, нарисованную на кнопке.</p>

<p>Ваш обработчик событий может выглядеть так:</p>

<pre>Public Sub Digit_Click()

&nbsp;&nbsp;Display = Display &amp; Last.Tag
&nbsp;&nbsp;RefreshDisplay
END</pre>


[LEFT]

<h3>Left$ / Mid$ / Right$</h3>

<p>Хорошо известные базовые процедуры <b>Left$</b>, <b>Right$</b>
и <b>Mid$</b> имеют полезное поведение в <b>Gambas</b></p>

<p>Второй параметр <b>Left$</b> и <b>Right$</b> является
опциональным и по умолчанию равен единице.</p>

<p><tt>Left$("Gambas")</tt> возвращает <tt>"G"</tt><br><tt>Right$("Gambas")</tt> возвращает <tt>"s"</tt></p>

<p>Этот второй параметр может быть отрицательным, а в таком случае
указывается количество символов, которые не нужно извлекать.</p>

<p><tt>Left$("Gambas", -2)</tt> возвращает <tt>"Gamb"</tt><br><tt>Right$("Gambas", -2)</tt> возвращает <tt>"mbas"</tt></p>

<p>Точно так же, третий аргумент <b>Mid$</b> может быть
отрицательным, а в таком случае указывается количество символов
в конце строки, которые не нужно извлекать.</p>

<p><tt>Mid$("Gambas", 2, -2)</tt> возвращает <tt>"amb"</tt></p>


[OBSERVER]

<h3>Наблюдатель</h3>

<p>Класс <b>Observer</b> позволяет вам перехватывать все события,
вызванные объектом, до того, как они действительно будут отправлены.</p>

<pre>MyTextBox = New TextBox(Me) As "MyTextBox"
MyObserver = New Observer(MyTextBox) As "MyObserver"
...
Public Sub MyObserver_KeyPress()
  Debug "Got it first"
End

Public Sub MyTextBox_KeyPress()
  Debug "Got it next"
End</pre>

<p>Наблюдатель может отменить событие с помощью <tt>Stop Event</tt>, чтобы не дать объекту эффективно
поднять его.</p>


[STRING]

<h3>Строки UTF-8</h3>

<p><b>Gambas</b> использует кодировку <b>UTF-8</b> для представления строк в памяти.</p>

<p>Но все стандартные строковые функции имеют дело только с <b>ASCII</b>:
<tt>Left</tt>, <tt>Mid</tt>, <tt>Right</tt>, <tt>UCase</tt>...</p>

<p>Если вы хотите работать со строками UTF-8, вы должны использовать методы
статического класса <b>String</b>, которые имеют то же имя, что и их стандартные аналоги.</p>

<pre>Print Len("bébé");; Left$("bébé", 3)
&rarr; 6 bé

Print String.Len("bébé");; String.Left("bébé", 3)
&rarr; 4 béb</pre>


[ASSIGNMENT]

<h3>Присваивания</h3>

<p><b>Gambas</b> реализует ярлыки присваивания, к которым привыкли программисты C/C ++.</p>

<pre>MyVariable += 2
MyVariable *= 4
MyVariable &amp;= "Great"</pre>
является эквивалентом этого:
<pre>MyVariable = MyVariable + 2
MyVariable = MyVariable * 4
MyVariable = MyVariable &amp; "Great"</pre>

<p>И так далее...</p>


[DEBUG]

<h3>Отладка</h3>

<p>Вы можете использовать инструкцию <b><tt>Debug</tt></b> для вывода
сообщений отладки в консоль (то есть стандартный вывод ошибок). Она
ведёт себя так же, как инструкция <tt>Print</tt>.</p>

<p>Эти сообщения имеют префикс имени класса, имени метода и номера строки
инструкции <tt>Debug</tt>. Если вам не нужен этот префикс, вы можете
использовать инструкцию <b><tt>Error</tt></b>
вместо <tt>Debug</tt>.</p>

<p>Сообщения отладки автоматически удаляются при создании исполняемого файла
без отладочной информации.</p>


[TRY]

<h3>Управление ошибками (1)</h3>

<p>Управление ошибками в <b>Gambas</b> осуществляется с помощью следующих инструкций:
<b><tt>Try</tt></b>, <b><tt>Error</tt></b>, <tt>Catch</tt> и <tt>Finally</tt>.</p>

<p><tt>Try</tt> пытается выполнить инструкцию, не вызывая ошибки. Ключевое слово <tt>Error</tt>
используется только для того, чтобы узнать, правильно ли был выполнен оператор.</p>

<pre>Try MyFile = Open "/etc/password" For Write
If Error Then Print "I cannot do what I want!"</pre>


[CATCH]

<h3>Управление ошибками (2)</h3>

<p>Управление ошибками в <b>Gambas</b> осуществляется с помощью следующих инструкций:
<tt>Try</tt>, <tt>Error</tt>, <b><tt>Catch</tt></b> и <tt>Finally</tt>.</p>

<p><tt>Catch</tt> указывает начало части управления ошибками функции или процедуры.
Он ставится в конце кода функции.</p>

<p>Часть catch выполняется при возникновении ошибки между началом
выполнения функции
и её концом.</p>

<p>Если возникает ошибка во время выполнения части catch, она нормально распространяется.</p>

<pre>Sub ProcessFile(FileName As String)
  ...
  Open FileName For Read As #hFile
  ...
  Close #hFile
  
Catch ' Выполняется только в случае ошибки

  Print "Cannot process file "; FileName

End</pre>


[FINALLY]

<h3>Управление ошибками (3)</h3>

<p>Управление ошибками в <b>Gambas</b> осуществляется с помощью следующих инструкций:
<tt>Try</tt>, <tt>Error</tt>, <tt>Catch</tt> и <b><tt>Finally</tt></b>.</p>

<p><tt>Finally</tt> представляет код, выполняемый в конце функции, даже если
во время её выполнения
возникла ошибка.</p>

<p>Часть finally не является обязательной. Если в функции есть часть catch, то часть finally должна предшествовать ей.</p>
 
<p>Если возникает ошибка во время выполнения части finally, она нормально распространяется.</p>

<pre>Sub ProcessFile(FileName As String)
  ...
  Open FileName For Read As #hFile
  ...
Finally ' Всегда выполняется, даже если возникла ошибка

  Close #hFile
  
Catch ' Выполняется только в случае ошибки
  
  Print "Cannot print file "; FileName
  
End</pre>


[OPTIONAL]

<h3>Опциональный</h3>

<p>Функции и процедуры в <b>Gambas</b> могут иметь опциональные аргументы.</p>

<p>Опциональные аргументы можно сделать, просто указав ключевое слово
<b><tt>Optional</tt></b> непосредственно перед именем аргумента.</p>

<p>Опциональные аргументы также могут иметь явное значение по умолчанию.</p>

<pre>Private Sub MyFunction(Param AS String, Optional Optim AS String = "Default")
  ...
  Print "Required: "; param; ", Optional: "; optim
  ...
End</pre>


[ARRAY]

<h3>For Each</h3>

<p>В <b>Gambas</b> вы можете легко проходить через массив, коллекцию или
множество других перечисляемых классов с помощью инструкции <b><tt>For Each</tt></b>.</p>

<p>Например:</p>

<pre>Dim Xml As New XmlDocument
Dim Node As XmlNode
Dim I As Integer

' Открыть файл XML
Xml.Open("pokus.xml")
' Потомки индексируются через [i], так как это массив
For I = 0 To Xml.Root.Children.Count - 1
  'Атрибуты зацикливаются через For Each, так как это коллекция
  For Each Node In Xml.Root.Children[i].Attributes
    Print Node.Name;; Node.Value
  Next
Next</pre>


[ICON]

<h3>Значки по умолчанию</h3>

<p>Вы можете использовать встроенные значки для более приятного графического
интерфейса вашего приложения, которые доступны в нескольких предопределённых
размерах (<tt>«маленький/small»</tt>, <tt>«средний/medium»</tt>, <tt>«большой/large»</tt>, ...) или абсолютных размерах (от 16x16 до 256x256).</p>

<p>Например:</p>

<pre>Image1.Picture = Picture["icon:/32/warning"]
Image2.Picture = Picture["icon:/small/error"]</pre>

<p><b>Предупреждение:</b> требуется компонент <tt>gb.form</tt>.</p>


[SETTINGS]

<h3>Параметры</h3>

<p>Если вам нужно сохранить конфигурацию вашей программы (например, геометрию
ваших форм), то вы счастливчик. Это очень легко и элегантно в <b>Gambas</b>. :-)</p>

<p>Чтобы сохранить положение формы:</p>
<pre>Settings.Write(TheForm)</pre>

<p>Для того, чтобы вспомнить:</p>
<pre>Settings.Read(TheForm)</pre>

Чтобы сохранить любые настройки:
<pre>Settings["Slot/Key"] = Value</pre>

И прочитать настройки обратно:
<pre>Value = Settings["Slot/Key", DefaultValue]</pre>

Эти настройки хранятся в файле <tt>~/.config/gambas3/&lt;MyApplication&gt;.conf</tt>,
где <tt>&lt;MyApplication&gt;</tt> -  это название вашего проекта.

<p><b>Предупреждение:</b> требуется компонент <tt>gb.settings</tt>.</p>


[EDITOR]

<p>Вот несколько советов по редактору...</p>

<h3>Два типа комментариев</h3>

<pre>' Нормальный комментарий</pre>
<b><pre>'' Жирный комментарий</pre></b>

<p>Жирные комментарии используются для документирования вашего кода.</p>

<h3>Как использовать фрагменты кода</h3>

<p>Давайте напечатаем <tt>main</tt>, затем нажмём клавишу <b>TAB</b>. Статическая
публичная функция запуска <tt>Main</tt> автоматически вставляется в ваш код.</p>

<p>Давайте напечатаем <tt>ds</tt>, затем нажмём клавишу <b>TAB</b>. Объявление
локальной строковой переменной вставляется автоматически, и вы можете сразу же ввести имя переменной.</p>

<p>Фрагменты кода полностью настраиваются в диалоговом окне «Предпочтения» меню «Инструменты» среды IDE.</p>


[END]

<h3>Это всё, друзья!</h3>

<p>Вы прочитали все полезные советы. Я надеюсь, что теперь вы
стали экспертом <b>Gambas</b>! :-)</p>

<p>Если вы хотите внести свой вклад, отправьте новые советы по следующему
адресу:</p>
<p><u>user@lists.gambas-basic.org</u></p>

<p>Заранее спасибо!</p>