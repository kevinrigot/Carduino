typedef struct RadioMessage
{
    int joystickX;
    int joystickY;
    boolean camMode;
};

class RadioMessageResponse{
  private:
    RadioMessage _message;
    boolean _isReceived = false;
  public:
    RadioMessageResponse(){}
    RadioMessageResponse(RadioMessage message){
      _message = message;
      _isReceived = true;
    }
    RadioMessage getRadioMessage(){
      return _message;
    }
    boolean isReceived(){
      return _isReceived;
    }
};

typedef struct RadioMessageToRemote
{
    float voltage1;
    float voltage2;
};
