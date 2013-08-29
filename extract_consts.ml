open Str

let show_me consts qui out_c out_ml =
  let lqui = String.lowercase qui in
  let lcqui = String.capitalize lqui in
  let l = List.rev (Hashtbl.find consts qui) in
  Printf.fprintf out_ml "type %s = \n" lqui;
  List.iter (fun (a, _, _) -> Printf.fprintf out_ml "  | %s\n" (String.capitalize
  (String.lowercase a))) l;
  Printf.fprintf out_ml "\n";
  Printf.fprintf out_c "const int fc%s[] = {" lcqui;
  List.iter (fun (_, _, c) -> Printf.fprintf out_c "%s, " c) l;
  Printf.fprintf out_c "};\n"

let read_fontconfig_h () =

  let dirs=
    let _ = Unix.putenv "PKG_CONFIG_ALLOW_SYSTEM_CFLAGS" "yes" in
    let cmd="pkg-config --cflags-only-I fontconfig" in
    try
      let p = Unix.open_process_in cmd in
      let res = input_line p in

      let ireg = regexp "-I\\([^ ]+\\)" in
      let rec split i sp=
        try
          let _ = search_forward ireg res i in
          split (match_end ()) ((matched_group 1 res) :: sp)
        with
          Not_found -> List.rev sp
      in

      if Unix.close_process_in p = Unix.WEXITED(0) then split 0 []
      else failwith (cmd^" failed")
    with
      | _ -> []
  in

  let rec find_file s paths=match paths with
      []->failwith (Printf.sprintf "file %S not found\n" s)
    | h::t->(
      let f=Filename.concat h s in
      if Sys.file_exists f then f else find_file s t
    )
  in
  let consts = Hashtbl.create 10
  and const_regexp = regexp "^#define\\b.*\\(FC_\\([^_]*\\)_\\([^ \t]*\\)\\)[ \t]+\\(.+\\)$"
  and file = open_in (find_file "fontconfig/fontconfig.h" dirs) in
    (try
      while true do
        let line = input_line file in
        if string_match const_regexp line 0 then
          let csname = matched_group 1 line
          and typ    = matched_group 2 line
          and cons   = matched_group 3 line
          and value  = matched_group 4 line in
          Hashtbl.replace consts typ ((cons, value, csname) ::
            (try Hashtbl.find consts typ with Not_found -> []))
      done
    with End_of_file -> ());
    consts

let _ =
  let consts = read_fontconfig_h ()
  and in_ml = open_in "fontconfig.ml.in"
  and lineno = ref 0
  and out_c = open_out "types.c"
  and subst_regexp = regexp "^[ \t]*(\\*\\*\\([A-Z]*\\)\\*)$"
  and out_ml = open_out "fontconfig.ml" in
  (* types.c header *)
  Printf.fprintf out_c "#include <fontconfig/fontconfig.h>\n";
  (* fontconfig.ml initial line number *)
  Printf.fprintf out_ml "#1 \"fontconfig.ml.in\"\n";
  (try
    while true do
      let line = input_line in_ml in
      incr lineno;
      if string_match subst_regexp line 0 then
        let qui = matched_group 1 line in
        (show_me consts qui out_c out_ml;
        Printf.fprintf out_ml "#%d \"%s\"\n" (!lineno + 1) "fontconfig.ml.in")
      else
        Printf.fprintf out_ml "%s\n" line
    done
  with End_of_file -> ());
