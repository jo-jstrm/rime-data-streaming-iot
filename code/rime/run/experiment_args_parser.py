from argparse import ArgumentParser


def create_parser() -> ArgumentParser:
    parser = ArgumentParser()
    parser.add_argument("--depth",
                        default=2,
                        help="The depth of the network tree",
                        type=int)
    parser.add_argument("--fanout",
                        default=8,
                        help="The number of leaves per node",
                        type=int)
    parser.add_argument("--sampling_interval",
                        default=500,
                        help="How fast to sample in millis",
                        type=int)
    parser.add_argument("--experiment_duration",
                        default=60,
                        help="Experiment duration, the system will sleep/run a bit longer than this value",
                        type=int)
    parser.add_argument("--send_query",
                        default=False,
                        action='store_true',
                        help="If the node is able to receive queries")
    parser.add_argument("--query_x",
                        default=0.0,
                        help="If the node is able to receive queries",
                        type=float)
    parser.add_argument("--query_y",
                        default=0.0,
                        help="If the node is able to receive queries",
                        type=float)
    parser.add_argument("--failure",
                        default=False,
                        action='store_true',
                        help="If the network will introduce failure")
    parser.add_argument("--movers",
                        default=0,
                        help="Size of set of nodes that are moving",
                        type=int)
    parser.add_argument("--baseline",
                        default="74797469/rime:latest",
                        help="Image of the baseline-compiled Rime",
                        type=str)
    parser.add_argument("--log_tuples",
                        default=False,
                        action='store_true',
                        help="Enable the logging of all data-tuples")
    return parser
