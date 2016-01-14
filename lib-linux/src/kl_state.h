///
/// @file kl_state.h
/// @author Kevin Lynx
/// @date 12.21.2009
/// @brief A simple FSM(finite state machine) pattern implemention.
///
#ifndef ___KL_STATE_H_
#define ___KL_STATE_H_

class state_machine;
class state
{
    public:
        virtual ~state() { }

        /// Receive an event to transform to other states.
        virtual void receive_event(state_machine*, void *pvoid) { }

        /// Execute when enter this state.
        virtual void enter( state_machine*) { }

        /// Execute when leave this state.
        virtual void leave( state_machine*) { }

    protected:
        void change(state_machine *machine, state *other);
};

class state_machine
{
    public:
        state_machine(state *pStartState)
        {
            clear();
            //_cur_state = pStartState;
            change(pStartState);
        }

        /// Clear the current state.
        void clear()
        {
            _cur_state = 0;
            _pre_state = 0;
        }

        /// Return true if the machine has a state.
        bool has_state() const
        {
            return _cur_state != 0;
        }

        /// Get the current state.
        state* cur_state()
        {
            return _cur_state;
        }

        void receive_event(void *pVoid)
        {
            if (_cur_state)
            {
                _cur_state->receive_event(this, pVoid);
            }
        }

    protected:
        friend class state;

        /// Change state.
        void change(state *other);

    private:
        state* _cur_state;
        state* _pre_state;
};

#endif // __KL_STATE_H_
