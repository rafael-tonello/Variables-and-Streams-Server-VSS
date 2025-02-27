this->init(){
    new promise this->_initRetProm
    _error="Not impelemnented"
    echo "$_error" > /dev/stderr
    this->_initRetProm->reject "$_error"; 
    _r="this->_initRetProm"
    return $retCode
}

this->checkout(){ local commit_or_tag="$1"; this->__chkout_onDone="$2"
    _error="IProjectAndSys::checkout is not impelemnented"
    echo "$_error" > /dev/stderr
    $this->__chkout_onDone 1 "$_error"
}

this->buildTests(){ this->__bldt_onDone="$1"
    _error="IProjectAndSys::buildTests is not  impelemnented"
    echo "$_error" > /dev/stderr
    $this->__bldt_onDone 1 "$_error"
}

this->runTests(){ this->__rnt_onDone="$1"
   _error="IProjectAndSys::runTests is not  impelemnented"
    echo "$_error" > /dev/stderr
    $this->__rnt_onDone 1 "$_error"
}

this->buildArtifacts(){ local customArtifactInfo=$1; this->__bldArt_onDone="$2"
    _error="IProjectAndSys::buildArtifacts is not  impelemnented"
    echo "$_error" > /dev/stderr
    $this->__bldArt_onDone 1 "$_error"
}

this->uploadArtifacts(){ local customArtifactInfo=$1; this->__uplArt_onDone="$2"
    _error="IProjectAndSys::uploadArtifacts is not  impelemnented"
    echo "$_error" > /dev/stderr
    $this->__uplArt_onDone 1 "$_error"
}

this->deployToProduction(){ this->__prdDepl_onDone="$1"
    _error="IProjectAndSys::deployToProduction is not  impelemnented"
    echo "$_error" > /dev/stderr
    $this->__prdDepl_onDone 1 "$_error"
}
