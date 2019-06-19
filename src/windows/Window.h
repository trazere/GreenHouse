
#ifndef Window_h
#define Window_h

/**
 * The window class implements the logic for driving a window.
 */
class Window {
  public:
    /**
     * Instantiates a new window.
     * 
     * description Description of the window.
     * directionPin Number of the direction control pin for the window.
     * motorPin Number of the motor control pin for the window.
     * openedPin Number of the opened sensor pin for the window.
     * closedPin Number of the closed sensor pin for the window.
     * moveTimeout Maximum duration of a move before autostopping (in milliseconds).
     */
    Window(const char *description, int directionPin, int motorPin, int openedPin, int closedPin, unsigned long moveTimeout);
    
    /**
     * Description of the window.
     */
    const char *description;
    
    /**
     * Sets things up for the window.
     */
    void setup();
    
    /**
     * Opens the window.
     * 
     * Does nothing when the window is open or already opening.
     */
    void open();
    
    /**
     * Closes the window.
     * 
     * Does nothing when the window is closed or already closing.
     */
    void close();
    
    /**
     * Stops the window.
     * 
     * Does nothing when the window is not moving.
     */
    void stop();
    
    /**
     * Stops the window when it has run its courses or after the move timeout has elapsed.
     * 
     * Does nothing when the window is not moving.
     */
    void autostop();
    
    /**
     * Indicates whether the window is open.
     * 
     * Returns TRUE when the window is open, FALSE otherwise.
     */
    bool isOpen();
    
    /**
     * Indicates whether the window is closed.
     * 
     * Returns TRUE when the window is closed, FALSE otherwise.
     */
    bool isClosed();
    
    /**
     * Indicates whether the window is currently moving.
     * 
     * Returns TRUE when the window is moving, FALSE otherwise.
     */
    bool isMoving();
    
    /**
     * Indicated whether timeout has elapsed during the last move.
     * 
     * Returns TRUC when the timeout has elapsed, FALSE otherwise.
     */
    bool hasTimedOut();
  
  private:
    int _directionPin;
    int _motorPin;
    int _openedPin;
    int _closedPin;
    unsigned long _moveTimeout;
    
    int _state;
    unsigned long _moveDeadline;
};

#endif
