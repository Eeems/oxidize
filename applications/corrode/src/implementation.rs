use interface::*;

/// A Greeting
pub struct Greeting {
    /// Emit signals to the Qt code.
    emit: GreetingEmitter,
    /// The message of the person.
    message: String,
}

/// Implementation of the binding
/// GreetingTrait is defined in interface.rs
impl GreetingTrait for Greeting {
    /// Create a new greeting with default data.
    fn new(emit: GreetingEmitter) -> Greeting {
        Greeting {
            emit: emit,
            message: "Hello World!",
        }
    }
    /// The emitter can emit signals to the Qt code.
    fn emit(&self) -> &GreetingEmitter {
        &self.emit
    }
    /// Get the message of the Greeting
    fn message(&self) -> &str {
        &self.message
    }
    /// Set the message of the Greeting
    fn set_message(&mut self, value: String) {
        self.message = value;
        self.emit.message_changed();
    }
}
