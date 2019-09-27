#include "controller.h"

Controller::Controller(QCoreApplication* application)
: QDBusAbstractAdaptor(application),
  app(application){

}
