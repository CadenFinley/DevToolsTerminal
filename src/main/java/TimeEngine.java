
import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 *
 * The ClockEngine class represents a timer or stopwatch that can be started and
 * stopped. It includes methods to start the clock, stop the clock, and return
 * the time in a formatted string.
 *
 * @author Caden Finley
 * @version 1.0
 */
public class TimeEngine {

    private boolean running = false;
    private long timeElapsedInSeconds = 0;
    private long remainingTimeInSeconds = 0;
    private final String whatAmI;
    private boolean hasTrigger = false;
    private final Object trigger;

    /**
     * Constructs a ClockEngine with the specified type.
     *
     * @param type the type of the clock engine
     * @param trigger the trigger object for synchronization
     */
    public TimeEngine(String type, Object trigger) {
        this.running = false;
        this.whatAmI = type;
        this.trigger = trigger;
        if (trigger != null) {
            hasTrigger = true;
        }
    }

    /**
     * Starts the clock with the specified period in seconds. Depending on the
     * type of clock (timer or stopwatch), it will start the appropriate
     * function.
     *
     * @param periodInSeconds the period in seconds for the clock to run
     */
    public void startClock(int periodInSeconds) {
        //System.out.println("Timer started");
        this.running = true;
        if (this.whatAmI.equals("timer")) {
            startTimer(periodInSeconds);
        }
        if (this.whatAmI.equals("stopwatch")) {
            startStopWatch(periodInSeconds);
        }
    }

    /**
     * Starts a timer for the specified length in seconds.
     *
     * @param lengthInSeconds the length of the timer in seconds
     */
    private void startTimer(int lengthInSeconds) {
        // Start a new thread to start the timer
        ScheduledExecutorService executor = Executors.newScheduledThreadPool(1);
        executor.scheduleAtFixedRate(() -> {
            if (this.running) {
                this.timeElapsedInSeconds++;
                if (this.timeElapsedInSeconds >= lengthInSeconds) {
                    //System.out.println("Timer stopped");
                    stopClock();
                    executor.shutdown();
                    if (hasTrigger) {
                        synchronized (trigger) {
                            trigger.notifyAll();
                        }
                    }
                }
                this.remainingTimeInSeconds = lengthInSeconds - this.timeElapsedInSeconds;
            } else {
                this.stopClock();
                executor.shutdown();
            }
        }, 0, 1, TimeUnit.SECONDS);
    }

    /**
     * Starts a stopwatch that increments the elapsed time in seconds at a fixed
     * rate.
     *
     * @param periodInSeconds the period in seconds at which the stopwatch
     * increments the elapsed time
     */
    private void startStopWatch(int periodInSeconds) {
        // Start a new thread to start the spotwatch
        ScheduledExecutorService executor = Executors.newScheduledThreadPool(1);
        executor.scheduleAtFixedRate(() -> {
            if (this.running) {
                this.timeElapsedInSeconds++;
            } else {
                this.stopClock();
                executor.shutdown();
            }
        }, 0, periodInSeconds, TimeUnit.SECONDS);
    }

    /**
     * Stops the clock by setting the running flag to false. This method can be
     * used to halt the clock's operation.
     */
    public void stopClock() {
        //System.out.println("Timer stopped");
        running = false;
    }

    /**
     * Returns the formatted time as a string in the format HH:MM:SS. If the
     * hours exceed 99, it caps the hours at 99. If the minutes exceed 59, it
     * caps the minutes at 59. If the seconds exceed 59, it caps the seconds at
     * 59.
     *
     * @return the formatted time string
     */
    public String returnTime() {
        if (whatAmI.equals("timer")) {
            long minutes = (this.remainingTimeInSeconds / 60) % 60;
            long hours = (this.remainingTimeInSeconds / 3600);
            return String.format("%02d:%02d:%02d", hours, minutes, this.remainingTimeInSeconds % 60);
        }
        long minutes = (this.timeElapsedInSeconds / 60) % 60;
        long hours = (this.timeElapsedInSeconds / 3600);
        if (hours >= 99) {
            hours = 99;
            if (minutes >= 59) {
                minutes = 59;
                if (this.timeElapsedInSeconds % 60 >= 59) {
                    this.timeElapsedInSeconds = 59;
                }
            }
        }
        return String.format("%02d:%02d:%02d", hours, minutes, this.timeElapsedInSeconds % 60);
    }

    /**
     * Gets the time elapsed in seconds.
     *
     * @return the time elapsed in seconds
     */
    public long getTimeElapsedInSeconds() {
        return this.timeElapsedInSeconds;
    }

    /**
     * Gets the remaining time in seconds.
     *
     * @return the remaining time in seconds
     */
    public long getRemainingTimeInSeconds() {
        return this.remainingTimeInSeconds;
    }

    /**
     * Sets the saved time in seconds.
     *
     * @param save the time to be saved in seconds
     */
    public void setSavedTimeInSeconds(long save) {
        this.timeElapsedInSeconds = save;
    }

    /**
     * Sets the debug time in seconds.
     *
     * @param time the debug time to be set in seconds
     */
    public void debugTime(long time) {
        this.timeElapsedInSeconds = time;
    }

    /**
     * Checks if the clock is running.
     *
     * @return true if the clock is running, false otherwise
     */
    public boolean isRunning() {
        return running;
    }

    /**
     * Adds time to the timer in seconds.
     *
     * @param timeInSeconds the time to add in seconds
     */
    public void addTimeToTimerInSeconds(int timeInSeconds) {
        if (whatAmI.equals("timer")) {
            this.remainingTimeInSeconds += timeInSeconds;
        }
    }

    /**
     * Returns the current timestamp as a formatted string.
     *
     * @return the current timestamp
     */
    public static String timeStamp() {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd_HH-mm-ss");
        Date resultDate = new Date(System.currentTimeMillis());
        return sdf.format(resultDate);
    }
}
