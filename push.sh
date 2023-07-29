#!/bin/sh
set -e
set_committer_to_author() {
[ "$#" -gt 0 ] || set -- '@{u}..'
FILTER_BRANCH_SQUELCH_WARNING=1 git filter-branch --setup '

# disable progress
report_progress() { :; }

envFilterMain() {
  # These variables must not be defined as local variables, or the export statement wont take effect.
  # local GIT_COMMITTER_DATE GIT_COMMITTER_NAME GIT_COMMITTER_EMAIL
  local at tz pid tmp pat pct ct

  case "${GIT_AUTHOR_DATE:?not defined or empty}" in @*" "[-+]????) ;; *)
    echo "unrecognized date format GIT_AUTHOR_DATE: \"$GIT_AUTHOR_DATE\"" >&2
    return 1
  esac
  tmp="${GIT_AUTHOR_DATE#@}"
  at="${tmp%% *}"
  tz="${tmp#* }"
  ct="$at"
  for pid in ${parents?not defined, the code of git-filter-branch changed}; do
    pid="$(map "$pid")" || return
    tmp="$(git log -1 --pretty="%at %ct" "$pid" --)" || return
    pat="${tmp%% *}"
    pct="${tmp#* }"
    [ "$pct" -ge "$ct" ] || continue
    if [ "$pat" -le "$at" ]; then
      ct="$pct"
    else
      ct="$((pct + 1))"
    fi
  done

  GIT_COMMITTER_DATE="@$ct $tz"
  GIT_COMMITTER_NAME="$GIT_AUTHOR_NAME"
  GIT_COMMITTER_EMAIL="$GIT_AUTHOR_EMAIL"
  export GIT_COMMITTER_DATE GIT_COMMITTER_NAME GIT_COMMITTER_EMAIL
}

' --env-filter envFilterMain --prune-empty -f -- "$@"
}
cd "${0%/*}"
git pull --rebase
set_committer_to_author
git push --force-with-lease zhanhb master
