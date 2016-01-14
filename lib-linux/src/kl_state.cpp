#include "kl_state.h"

void state::change(state_machine *machine, state *other)
{
    machine->change(other);
}


void state_machine::change(state *other)
{
    if( has_state() )
    {
        _cur_state->leave(this);
    }
    other->enter(this);
    _pre_state = _cur_state;
    _cur_state = other;
}
