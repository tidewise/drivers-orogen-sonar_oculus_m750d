# frozen_string_literal: true

using_task_library "sonar_oculus_m750d"

using_task_library "iodrivers_base"
require "iodrivers_base/orogen_test_helpers"

describe OroGen.sonar_oculus_m750d.Task do
    include IODriversBase::OroGenTestHelpers

    run_live

    attr_reader :task, :from_driver, :to_driver, :pressure_w

    before do
        @task = syskit_deploy(
            OroGen.sonar_oculus_m750d.Task.deployed_as("test")
        )

        @raw_io = setup_iodrivers_base_with_ports(@task, configure_and_start: false)
        @from_driver = syskit_create_reader @raw_io.in_port, type: :buffer, size: 20
        @to_driver = syskit_create_writer @raw_io.out_port, type: :buffer, size: 20
        @pressure_w = syskit_create_writer @task.pressure_port

        @task.properties.pressure_data_timeout = { microseconds: 1_000_000 }
    end

    describe "start behaviour" do
        it "can disable pressure safety check" do
            task.properties.safe_working_pressure = { pascal: Float::NAN }
            syskit_configure(task)
            packet =
                expect_execution { task.start! }
                .to do
                    emit task.start_event
                    not_emit task.unsafe_working_pressure_event, within: 2
                    have_one_new_sample from_driver
                end

            assert_fire_sonar_message(packet)
        end

        it "goes into unsafe_working_pressure state if the check is enabled and " \
           "there are no pressure samples" do
            task.properties.safe_working_pressure = { pascal: 100_000 }
            syskit_configure(task)
            expect_execution { task.start! }
                .to do
                    emit task.unsafe_working_pressure_event
                    have_no_new_sample from_driver
                end
        end

        it "goes into unsafe_working_pressure state if the check is enabled and it " \
           "receives unsafe pressure samples" do
            task.properties.safe_working_pressure = { pascal: 150_000 }
            syskit_configure(task)

            expect_execution { task.start! }
                .poll { pressure_w.write({ pascal: 100_000 })}
                .to do
                    emit task.unsafe_working_pressure_event
                    have_no_new_sample from_driver
                end
        end

        it "goes into normal state if the check is enabled and it " \
           "receives safe pressure samples" do
            task.properties.safe_working_pressure = { pascal: 150_000 }
            syskit_configure(task)
            packet =
                expect_execution { task.start! }
                .poll { pressure_w.write({ pascal: 200_000 })}
                .to do
                    emit task.start_event
                    not_emit task.unsafe_working_pressure_event, within: 1
                    have_one_new_sample from_driver
                end

            assert_fire_sonar_message(packet)
        end
    end

    it "switches from safe to unsafe at runtime" do
        task.properties.safe_working_pressure = { pascal: 150_000 }
        syskit_configure(task)
        expect_execution { task.start! }
            .poll { pressure_w.write({ pascal: 200_000 })}
            .to do
                emit task.start_event
                not_emit task.unsafe_working_pressure_event, within: 0.5
                have_one_new_sample from_driver
            end

        expect_execution
            .poll { pressure_w.write({ pascal: 100_000 })}
            .to_emit task.unsafe_working_pressure_event
    end

    it "switches from unsafe to safe at runtime" do
        task.properties.safe_working_pressure = { pascal: 150_000 }
        syskit_configure(task)
        expect_execution { task.start! }
            .poll { pressure_w.write({ pascal: 100_000 })}
            .to_emit task.unsafe_working_pressure_event

        packet =
            expect_execution
            .poll { pressure_w.write({ pascal: 200_000 })}
            .to do
                emit task.running_event
                have_one_new_sample from_driver
            end
        assert_fire_sonar_message(packet)
    end

    def assert_fire_sonar_message(packet)
        assert_equal "\x15\x00", raw_packet_to_s(packet)[6, 2]
    end
end