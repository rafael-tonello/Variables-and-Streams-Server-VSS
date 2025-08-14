inherits "IProjectAnsSys" "$this->name"

this->init_r=0
this->init_error=""
this->init_delay=1
this->init(){
    new promise this->_initRetProm
    scheduler->runDelayed _(){
        if [ "$this->init_error" != "" ]; then
            _error="$this->init_error"
            this->_initRetProm->reject "$_error"; 
        else
            this->_initRetProm->resolve 0
        fi
    }()_ $this->init_delay
    _r="this->_initRetProm"
    return 0
}


this->checkout_r=0
this->checkout_error=""
this->checkout_delay=1
this->checkout(){ local commit_or_tag="$1"; this->__chkout_onDone="$2"
    scheduler->runDelayed _(){
        _r="$this->checkout_r"
        _error="$this->checkout_error"
        $this->__chkout_onDone $_r "$_error"
    }()_ $this->checkout_delay
}


this->buildTests_r=0
this->buildTests_error=""
this->buildTests_delay=1
this->buildTests(){ this->__bldt_onDone="$1"
    scheduler->runDelayed _(){
        _r="$this->buildTests_r"
        _error="$this->buildTests_error"
        $this->__bldt_onDone $_r "$_error"
    }()_ $this->buildTests_delay
}

this->runTests_r=0
this->runTests_error=""
this->runTests_delay=1
this->runTests(){ this->__rnt_onDone="$1"
    scheduler->runDelayed _(){
        _r="$this->runTests_r"
        _error="$this->runTests_error"
        $this->__rnt_onDone $_r "$_error"
    }()_ $this->runTests_delay
}

this->buildArtifacts_r=0
this->buildArtifacts_error=""
this->buildArtifacts_delay=1
this->buildArtifacts(){ local customArtifactInfo=$1; this->__bldArt_onDone="$2"
    scheduler->runDelayed _(){
        _r="$this->buildArtifacts_r"
        _error="$this->buildArtifacts_error"
        $this->__bldArt_onDone $_r "$_error"
    }()_ $this->buildArtifacts_delay
}

this->uploadArtifacts_r=0
this->uploadArtifacts_error=""
this->uploadArtifacts_delay=1
this->uploadArtifacts(){ local customArtifactInfo=$1; this->__uplArt_onDone="$2"
    echo 1
    echo " the delay to be used is $this->uploadArtifacts_delay"
    scheduler->runDelayed _(){
        _r="$this->uploadArtifacts_r"
        _error="$this->uploadArtifacts_error"
        echo 3
        $this->__uplArt_onDone $_r "$_error"
    }()_ $this->uploadArtifacts_delay
}

this->deployToProduction_r=0
this->deployToProduction_error=""
this->deployToProduction_delay=1
this->deployToProduction(){ local onDone="$1"; this->__prdDepl_onDone="$2"
    scheduler->runDelayed _(){
        _r="$this->deployToProduction_r"
        _error="$this->deployToProduction_error"
        $this->__prdDepl_onDone $_r "$_error"
    }()_ $this->deployToProduction_delay
}
