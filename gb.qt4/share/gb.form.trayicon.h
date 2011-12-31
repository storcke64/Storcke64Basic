/***************************************************************************

  gb.form.trayicon.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

static const char * _default_trayicon[] = {
"24 24 136 2",
"  	c None",
". 	c #4167C6",
"+ 	c #446ECF",
"@ 	c #4975D8",
"# 	c #4E7DDF",
"$ 	c #5586E7",
"% 	c #314F9E",
"& 	c #2D4A91",
"* 	c #3153A9",
"= 	c #304F9B",
"- 	c #365AB7",
"; 	c #3C64C5",
"> 	c #446FD3",
", 	c #4D7DE1",
"' 	c #5A8EED",
") 	c #2D4371",
"! 	c #46628B",
"~ 	c #688CB7",
"{ 	c #5D7FAA",
"] 	c #668AB6",
"^ 	c #3B5CA7",
"/ 	c #3556A2",
"( 	c #365BB8",
"_ 	c #3F67C9",
": 	c #4977DC",
"< 	c #568AEB",
"[ 	c #33496F",
"} 	c #749CC7",
"| 	c #80AAD7",
"1 	c #81ADD9",
"2 	c #82AED9",
"3 	c #81ACD8",
"4 	c #7AA4D0",
"5 	c #729BCA",
"6 	c #3457AE",
"7 	c #476AB0",
"8 	c #577CB9",
"9 	c #4870C1",
"0 	c #5486E6",
"a 	c #7BA4D0",
"b 	c #7DA8D5",
"c 	c #7AA5D3",
"d 	c #80ABD8",
"e 	c #82AEDA",
"f 	c #83AFDA",
"g 	c #7EA8D6",
"h 	c #668CC1",
"i 	c #7EA9D5",
"j 	c #81ADD8",
"k 	c #8CB5DC",
"l 	c #9FBDDC",
"m 	c #BBD7ED",
"n 	c #9AAEC3",
"o 	c #79A2D1",
"p 	c #77A1D1",
"q 	c #5278BB",
"r 	c #3D5DA1",
"s 	c #6F97C8",
"t 	c #587DBA",
"u 	c #7EA9D6",
"v 	c #82ADDA",
"w 	c #81ACD9",
"x 	c #7FAAD6",
"y 	c #6F91B9",
"z 	c #6A85A0",
"A 	c #9DC4E5",
"B 	c #849AA4",
"C 	c #1E1F20",
"D 	c #6189C5",
"E 	c #335195",
"F 	c #395BA9",
"G 	c #4667A2",
"H 	c #4E72B6",
"I 	c #2E4E9F",
"J 	c #729ACA",
"K 	c #7FAAD7",
"L 	c #83AFDB",
"M 	c #7CA6D3",
"N 	c #202B38",
"O 	c #020202",
"P 	c #495B6D",
"Q 	c #C4E6FB",
"R 	c #718186",
"S 	c #4A6EB5",
"T 	c #3E5E9E",
"U 	c #587DBC",
"V 	c #78A1CF",
"W 	c #466AB6",
"X 	c #5F84BB",
"Y 	c #78A0CE",
"Z 	c #7FA9D6",
"` 	c #24313F",
" .	c #324256",
"..	c #97C1E6",
"+.	c #C7E9FC",
"@.	c #2F4E98",
"#.	c #719ACA",
"$.	c #6990C2",
"%.	c #81ABD6",
"&.	c #78A0CA",
"*.	c #5E7DA0",
"=.	c #77A0CB",
"-.	c #80ABD7",
";.	c #93BEE4",
">.	c #C4E5FA",
",.	c #4B6DA9",
"'.	c #6F97C5",
").	c #84AFDA",
"!.	c #82ADD9",
"~.	c #8AB5DE",
"{.	c #B0D6F1",
"].	c #719ACC",
"^.	c #7197C3",
"/.	c #85B0DB",
"(.	c #84B0DB",
"_.	c #91BCE2",
":.	c #638DD3",
"<.	c #799DDB",
"[.	c #4A6EC2",
"}.	c #718AB8",
"|.	c #8DA6CA",
"1.	c #92ACD1",
"2.	c #C4D9EE",
"3.	c #AEC6E7",
"4.	c #38539B",
"5.	c #405A99",
"6.	c #7D95BC",
"7.	c #9EB6D7",
"8.	c #C3D8EE",
"9.	c #5771AD",
"0.	c #25418F",
"a.	c #4863A1",
"b.	c #C5D9EF",
"c.	c #6384CA",
"d.	c #3D65C5",
"e.	c #395AB0",
"                                                ",
"                                                ",
"            . + @ # $                           ",
"        % & * = - ; > , '                       ",
"      ) ! ~ { ] ^ / ( _ : <                     ",
"    [ } | 1 2 3 4 5 6 7 8 9 0                   ",
"    a b c d 1 e f g h i j k l m n               ",
"  o p q r s t u v w f v x y z A B C             ",
"  D E F G H I J K f L e M N O P Q R             ",
"  S T U V W X Y u f L f Z ` O  ...+.            ",
"  @.#.$.        %.e L L e &.*.=.-.;.>.          ",
"  ,.'.              ).L L f e e e !.~.{.        ",
"  ].^.                  /.(.L L f f f f _.      ",
"  b                                             ",
"  -.                                            ",
"                                                ",
"    :.                                          ",
"    <.[.}.|.1.                                  ",
"    2.3.4.5.6.7.                                ",
"      8.9.0.a.                                  ",
"      b.c.d.e.                                  ",
"                                                ",
"                                                ",
"                                                "};


 
