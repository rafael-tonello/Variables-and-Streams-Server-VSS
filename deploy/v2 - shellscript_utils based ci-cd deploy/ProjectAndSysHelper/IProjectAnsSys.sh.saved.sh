__app___ProjAndSysHelper_init(){
    new promise __app___ProjAndSysHelper__initRetProm
    _error="Not impelemnented"
    echo "$_error" > /dev/stderr
    __app___ProjAndSysHelper__initRetProm_reject "$_error"; 
    _r="__app___ProjAndSysHelper__initRetProm"
    return $retCode
}

__app___ProjAndSysHelper_checkout(){ local commit_or_tag="$1"; __app___ProjAndSysHelper___chkout_onDone="$2"
    _error="IProjectAndSys::checkout is not impelemnented"
    echo "$_error" > /dev/stderr
    $__app___ProjAndSysHelper___chkout_onDone 1 "$_error"
}

__app___ProjAndSysHelper_buildTests(){ __app___ProjAndSysHelper___bldt_onDone="$1"
    _error="IProjectAndSys::buildTests is not  impelemnented"
    echo "$_error" > /dev/stderr
    $__app___ProjAndSysHelper___bldt_onDone 1 "$_error"
}

__app___ProjAndSysHelper_runTests(){ __app___ProjAndSysHelper___rnt_onDone="$1"
   _error="IProjectAndSys::runTests is not  impelemnented"
    echo "$_error" > /dev/stderr
    $__app___ProjAndSysHelper___rnt_onDone 1 "$_error"
}

__app___ProjAndSysHelper_buildArtifacts(){ local customArtifactInfo=$1; __app___ProjAndSysHelper___bldArt_onDone="$2"
    _error="IProjectAndSys::buildArtifacts is not  impelemnented"
    echo "$_error" > /dev/stderr
    $__app___ProjAndSysHelper___bldArt_onDone 1 "$_error"
}

__app___ProjAndSysHelper_uploadArtifacts(){ local customArtifactInfo=$1; __app___ProjAndSysHelper___uplArt_onDone="$2"
    _error="IProjectAndSys::uploadArtifacts is not  impelemnented"
    echo "$_error" > /dev/stderr
    $__app___ProjAndSysHelper___uplArt_onDone 1 "$_error"
}

__app___ProjAndSysHelper_deployToProduction(){ __app___ProjAndSysHelper___prdDepl_onDone="$1"
    _error="IProjectAndSys::deployToProduction is not  impelemnented"
    echo "$_error" > /dev/stderr
    $__app___ProjAndSysHelper___prdDepl_onDone 1 "$_error"
}
