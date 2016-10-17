function remove_quotes(str) {
  return substr(str, 2, length(str) - 2)
}

function trim_project(str) {
  split(remove_quotes(str), pieces, "/")
  return pieces[2]
}

($1 == "git" || $1 == "github") && (NF == 4) {
  project=trim_project($2)
  constraint=$3
  version=$4

  print project, constraint, version
}

($1 == "git" || $1 == "github") && (NF == 3) {
  project=trim_project($2)
  version=remove_quotes($3)

  print project, "==", version
}
