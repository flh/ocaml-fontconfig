open Fontconfig;;
open Pattern;;
open Properties;;

let test l =
  match Pattern.get (find_font (Properties.make l)) "file" 0 with
  String s -> Printf.printf "%s\n" s
  | _ -> Printf.printf "shouldn't happen\n";;

test [Family ("Alegreya"); Style "Bold Italic"];;
test [Family ("Asana Matssh")];;
test [Family ("Asana Math")];;
test [Family ("Alegreya"); Slant Italic];;
test [Family ("Alegreya"); Slant Italic; Weight Bold];;
