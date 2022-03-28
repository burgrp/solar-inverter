class AC {

volatile target::tc::Peripheral* tc;

    public:

    void init(volatile target::tc::Peripheral* tc) {
        this->tc = tc;

        //tc->COUNT32.CTRLA
    }

    void interruptHandlerTC() {
    }
};