using_task_library "sonar_oculus_m750d"

describe OroGen.sonar_oculus_m750d.Task do
    run_live

    attr_reader :task
    before do
        @task = syskit_deploy(
            OroGen.sonar_oculus_m750d.Task.deployed_as("sonar_oculus_m750d_driver")
        )
        syskit_start_execution_agents(@task)

        # "connect" the io_raw_in_port so it gets configured without the io_port
        # property set
        writer = syskit_create_writer(@task.io_raw_in_port)
    end

    it "can disable pressure safety check" do
        task.properties.safe_working_pressure = Types.base.Pressure.new(pascal: Float::NAN)
        syskit_configure(task)
        expect_execution do
            task.start!
        end.to do
            emit task.start_event
            not_emit task.unsafe_working_pressure_event, within: 2
        end
    end

    it "can enable pressure safety check a first time" do
        syskit_configure(task)
        expect_execution do
            task.start!
        end.to do
            emit task.start_event
            emit task.unsafe_working_pressure_event
        end
    end
end