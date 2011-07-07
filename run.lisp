(defparameter *lcos-chan* nil)
(defun lcos (cmd)
  (let ((s (sb-ext:process-input *lcos-chan*)))
    (format s "~a~%" cmd)
    (finish-output s)))

(progn
  (setf *lcos-chan*
	(sb-ext:run-program "/home/martin/0630/glfw/glfw" '("400" "420")
			    :output :stream
			    :input :stream
			    :wait nil))
  
  (sb-thread:make-thread 
   #'(lambda ()
       (unwind-protect
           (with-open-stream (s (sb-ext:process-output *lcos-chan*))
             (loop for line = (read-line s nil nil)
                while line do
                  (format t "lcos read: ~a~%" line)
                  (finish-output)))
	 (sb-ext:process-close *lcos-chan*)))
   :name "cmd-reader"))

(lcos "toggle-stripes 0")

(lcos "quit")

(let ((a (random 100)))
 (dotimes (i 400)
   (lcos (format nil "qline 0 ~a ~a 200" a i))
   (lcos "qswap")))
